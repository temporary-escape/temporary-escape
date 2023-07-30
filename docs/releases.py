import boto3
import datetime
import math
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Dict
from urllib.parse import urlparse

DIR = os.path.dirname(os.path.realpath(__file__))


@dataclass
class Content:
    os: str
    url: str
    filename: str
    size: int
    kind: str


@dataclass
class Release:
    version: str
    created: datetime.datetime
    files: List[Content]


def get_env(key: str) -> str:
    value = os.getenv(key)
    if not value:
        print(f"Missing env variable: {key}")
        sys.exit(1)
    return value


def filter_files_only(data: dict) -> bool:
    return not data["Key"].endswith("/")


def get_latest_release(versions: List[dict]) -> dict:
    for version in versions:
        if "-" not in version["version"]:
            return version
    assert False, "Unable to find exact tag version"


def get_os_type(file: str) -> str:
    tokens = file.rsplit("-", maxsplit=1)
    assert len(tokens) == 2, f"Bad filename: {file}"

    tokens = tokens[1].split(".", maxsplit=1)
    assert len(tokens) == 2, f"Bad filename: {file}"

    platform_key = tokens[0]
    if platform_key == "Darwin":
        platform_key = "MacOS"
    return platform_key


def to_download_link(endpoint: str, bucket: str, file: str):
    o = urlparse(endpoint)
    return f"{o.scheme}://{bucket}.{o.netloc}/{file}"


def get_file_type(file: str):
    if file.endswith(".zip"):
        return "Portable ZIP"
    if file.endswith(".msi"):
        return "Installer"
    if file.endswith(".tar.gz"):
        return "TAR Archive"
    if file.endswith(".AppImage"):
        return "AppImage"
    if file.endswith(".dmg"):
        return "Apple Disk Image"
    assert False, f"Unknown file type: {file}"


def process_versions(endpoint: str, bucket: str, versions: List[dict]):
    processed = []

    for version in versions:
        release = Release(
            version=version["version"],
            created=version["created"],
            files=[],
        )

        for file, size in version["files"]:
            release.files.append(
                Content(
                    os=get_os_type(file),
                    url=to_download_link(endpoint, bucket, file),
                    filename=file.rsplit("/", maxsplit=1)[1],
                    size=size,
                    kind=get_file_type(file),
                )
            )

        processed.append(release)

    return processed


def process_files(endpoint: str, bucket: str, files: List[dict]):
    # Sort by modified time
    files = reversed(sorted(files, key=lambda f: f["LastModified"]))

    versions = []
    current_version = None

    # Extract version tag and group by version
    last_version = None
    for file in files:
        tokens = file["Key"].rsplit("/", maxsplit=2)
        assert len(tokens) == 3, f"Bad S3 object file: {file}"

        if last_version != tokens[1]:
            if current_version:
                versions.append(current_version)
            current_version = {
                "version": tokens[1],
                "files": [],
                "created": file["LastModified"],
            }
            last_version = tokens[1]

        current_version["files"].append((file["Key"], file["Size"]))

    if len(current_version) > 0:
        versions.append(current_version)

    return process_versions(endpoint, bucket, versions)


def search_release(s3, endpoint: str, bucket: str):
    list_objects_v2 = s3.get_paginator("list_objects_v2")

    starting_token = None
    paginator = list_objects_v2.paginate(
        Bucket=bucket,
        Prefix="release/",
        PaginationConfig={
            "PageSize": 20,
            "StartingToken": starting_token,
        },
    )

    files = []
    for page in paginator:
        files.extend(list(filter(filter_files_only, page["Contents"])))

    return process_files(endpoint, bucket, files)


def get_latest_release(releases: List[Release]) -> Release:
    for release in releases:
        if "-" not in release.version:
            return release
    assert False, "Unable to find exact tag version"


def get_windows_exe(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "Windows" and f.kind == "Installer"), None
    )


def get_linux_exe(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "Linux" and f.kind == "AppImage"), None
    )


def get_macos_exe(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "MacOS" and f.kind == "Apple Disk Image"),
        None,
    )


def get_windows_zip(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "Windows" and f.kind == "Portable ZIP"), None
    )


def get_linux_zip(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "Linux" and f.kind == "TAR Archive"), None
    )


def get_macos_zip(release: Release) -> Content:
    return next(
        (f for f in release.files if f.os == "MacOS" and f.kind == "Portable ZIP"),
        None,
    )


def convert_size(size_bytes: int) -> str:
    """
    Source: https://stackoverflow.com/a/14822210
    """

    if size_bytes == 0:
        return "0B"
    size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
    i = int(math.floor(math.log(size_bytes, 1024)))
    p = math.pow(1024, i)
    s = round(size_bytes / p, 2)
    return "%s %s" % (s, size_name[i])


def replace_all(content: str, replacements: Dict[str, str]) -> str:
    for key, value in replacements.items():
        content = content.replace(key, value)
    return content


def create_md_table_rows(releases: List[Release]) -> str:
    content = ""
    for release in releases:
        content += "| {} | {:%B %d, %Y} | ".format(release.version, release.created)

        first = True
        for os in ["Windows", "Linux", "MacOS"]:
            for file in release.files:
                if file.os == os:
                    if first == True:
                        first = False
                    else:
                        content += ",<br>"
                    content += f"[{file.os} {file.kind}]({file.url}) ({convert_size(file.size)})"
        content += " |\n"
    return content


def replace_content(path: str, releases: List[Release]):
    latest = get_latest_release(releases)

    replacements = {
        "DOWNLOAD_WINDOWS_LATEST_EXE_URL": get_windows_exe(latest).url,
        "DOWNLOAD_LINUX_LATEST_EXE_URL": get_linux_exe(latest).url,
        "DOWNLOAD_MACOS_LATEST_EXE_URL": get_macos_exe(latest).url,
        "DOWNLOAD_WINDOWS_LATEST_EXE_SIZE": convert_size(get_windows_exe(latest).size),
        "DOWNLOAD_LINUX_LATEST_EXE_SIZE": convert_size(get_linux_exe(latest).size),
        "DOWNLOAD_MACOS_LATEST_EXE_SIZE": convert_size(get_macos_exe(latest).size),
        "DOWNLOAD_WINDOWS_LATEST_ZIP_URL": get_windows_zip(latest).url,
        "DOWNLOAD_LINUX_LATEST_ZIP_URL": get_linux_zip(latest).url,
        "DOWNLOAD_MACOS_LATEST_ZIP_URL": get_macos_zip(latest).url,
        "DOWNLOAD_WINDOWS_LATEST_ZIP_SIZE": convert_size(get_windows_zip(latest).size),
        "DOWNLOAD_LINUX_LATEST_ZIP_SIZE": convert_size(get_linux_zip(latest).size),
        "DOWNLOAD_MACOS_LATEST_ZIP_SIZE": convert_size(get_macos_zip(latest).size),
        "DOWNLOAD_ALL_TABLE": create_md_table_rows(releases),
        "LATEST_VERSION_STR": latest.version,
        "LATEST_VERSION_DATE": "{:%B %d, %Y}".format(latest.created),
    }

    for file in Path(path).rglob("*.md.in"):
        with open(file, "r") as f:
            content = f.read()

        content = replace_all(content, replacements)

        with open(str(file).replace(".in", ""), "w") as f:
            f.write(content)


def main():
    if os.getenv("AWS_ACCESS_KEY_ID") is None:
        print("Skipping, no AWS S3 credentials supplied")
        return

    session = boto3.session.Session()
    endpoint_url = "https://{}".format(get_env("AWS_ENDPOINT_URL"))
    s3 = session.client(
        service_name="s3",
        aws_access_key_id=get_env("AWS_ACCESS_KEY_ID"),
        aws_secret_access_key=get_env("AWS_SECRET_ACCESS_KEY"),
        endpoint_url=endpoint_url,
    )

    releases = search_release(s3, endpoint_url, get_env("AWS_BUCKET_NAME"))
    replace_content(os.path.join(DIR, "source"), releases)


if __name__ == "__main__":
    main()
