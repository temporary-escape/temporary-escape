import os
import sys
from argparse import ArgumentParser
from typing import List
from urllib.parse import urlparse

import boto3

MARKDOWN_START = """
---
weight: 1
title: "Download"
---

# Download

"""

SYSTEM_REQUIREMENTS = """
## System Requirements

These are the minimum system requirements.
You must have Vulkan 1.1 (or higher) compatible graphics card and a 64-bit operating system.

{{< tabs "system-requirements" >}}
{{< tab "Minimum" >}}
* **OS:** Windows 10 64-bit, Ubuntu/Debian/Manjaro 64-bit, or MacOS
* **Display:** 1680x1050
* **Processor:** Intel Core i5 or AMD Ryzen 3
* **Memory:** 4 GB RAM
* **Graphics:** Intel HD Graphics 500, NVidia GTX 650 Ti, or AMD Radeon R7
* **Vulkan:** 1.1 minimum
* **Storage:** 500 MB available space
{{< /tab >}}
{{< tab "Recommended" >}}
* **OS:** Windows 10 64-bit, Ubuntu/Debian/Manjaro 64-bit, or MacOS
* **Display:** 1920x1080
* **Processor:** Intel Core i7 or AMD Ryzen 5
* **Memory:** 8 GB RAM
* **Graphics:** NVidia GTX 1050 Ti or AMD Radeon RX 5600
* **Vulkan:** 1.1 minimum
* **Storage:** 2 GB available space
{{< /tab >}}
{{< /tabs >}}
"""


def to_download_link(endpoint: str, bucket: str, file: str):
    o = urlparse(endpoint)
    return f'{o.scheme}://{bucket}.{o.netloc}/{file}'


def get_file_type(file: str):
    if file.endswith('.zip'):
        return 'Portable ZIP'
    if file.endswith('.msi'):
        return 'Installer'
    if file.endswith('.tar.gz'):
        return 'TAR Archive'
    assert False, f'Unknown file type: {file}'


def get_file_name(file: str):
    return file.rsplit('/', maxsplit=1)[-1]


def create_markdown_version(s3, endpoint: str, bucket: str, version: dict, id=None):
    platforms = {
        'Windows': [],
        'Linux': [],
        'MacOS': [],
    }

    for file in version['files']:
        tokens = file.rsplit('-', maxsplit=1)
        assert len(tokens) == 2, f'Bad filename: {file}'

        tokens = tokens[1].split('.', maxsplit=1)
        assert len(tokens) == 2, f'Bad filename: {file}'

        platforms[tokens[0]].append(to_download_link(endpoint, bucket, file))

    tag = version['version'] if id is None else id

    print(f'{{{{< tabs "{tag}" >}}}}')
    for platform, files in platforms.items():
        print(f'{{{{< tab "{platform}" >}}}}')
        print('**Files:**\n')
        for file in files:
            print(f' * {get_file_type(file)}: [{get_file_name(file)}]({file})\n')
        print('{{< /tab >}}')
    print('{{< /tabs >}}')


def get_latest_release(versions: List[dict]) -> dict:
    for version in versions:
        if '-' not in version['version']:
            return version
    assert False, 'Unable to find exact tag version'


def create_markdown(s3, endpoint: str, bucket: str, versions: List[dict]):
    print(MARKDOWN_START)

    latest = get_latest_release(versions)
    print('## Latest Releases\n')

    print('_Version: {} ({:%B %d, %Y})_\n'.format(latest['version'], latest['created']))
    create_markdown_version(s3, endpoint, bucket, latest, id='latest')

    print(SYSTEM_REQUIREMENTS)

    print('## All Releases\n')
    print('All releases list chronologically including development releases\n')
    for version in versions:
        print('{{{{< expand "{}" >}}}}\n'.format(version['version']))
        print('_Version: {} ({:%B %d, %Y})_\n'.format(version['version'], version['created']))
        create_markdown_version(s3, endpoint, bucket, version)
        print('{{< /expand >}}')


def process_versions(s3, endpoint: str, bucket: str, files: List[str]):
    # Sort by modified time
    files = reversed(sorted(files, key=lambda f: f['LastModified']))

    versions = []
    current_version = None

    # Extract version tag and group by version
    last_version = None
    for file in files:
        tokens = file['Key'].rsplit('/', maxsplit=2)
        assert len(tokens) == 3, f'Bad S3 object file: {file}'

        if last_version != tokens[1]:
            if current_version:
                versions.append(current_version)
            current_version = {
                'version': tokens[1],
                'files': [],
                'created': file['LastModified'],
            }
            last_version = tokens[1]

        current_version['files'].append(file['Key'])

    if len(current_version) > 0:
        versions.append(current_version)

    # versions now contains list of dictionaries with version string and list of files
    # print(versions)
    create_markdown(s3, endpoint, bucket, versions)


def filter_files_only(data: dict) -> bool:
    return not data['Key'].endswith('/')


def search_release(s3, endpoint: str, bucket: str):
    list_objects_v2 = s3.get_paginator('list_objects_v2')

    starting_token = None
    paginator = list_objects_v2.paginate(
        Bucket=bucket,
        Prefix='release/',
        PaginationConfig={
            'PageSize': 20,
            'StartingToken': starting_token,
        })

    files = []
    for page in paginator:
        files.extend(list(filter(filter_files_only, page['Contents'])))

    process_versions(s3, endpoint, bucket, files)


def main(argv: List[str]):
    parser = ArgumentParser()
    parser.add_argument('--endpoint', type=str, required=True)
    parser.add_argument('--bucket', type=str, required=True)
    args = parser.parse_args(argv)

    session = boto3.session.Session()
    s3 = session.client(
        service_name='s3',
        aws_access_key_id=os.getenv('AWS_ACCESS_KEY_ID'),
        aws_secret_access_key=os.getenv('AWS_SECRET_ACCESS_KEY'),
        endpoint_url=args.endpoint,
    )

    search_release(s3, args.endpoint, args.bucket)


if __name__ == '__main__':
    main(sys.argv[1::])
