function githubGetReleases(owner, repo) {
    fetch('https://api.github.com/repos/' + owner + '/' + repo + '/releases')
        .then(response => response.json())
        .then(response => {
            console.log(response)

            const target = document.getElementById('latest-releases');
            document.getElementById('loading').remove();

            for (const release of response.reverse()) {
                const span = document.createElement('span');
                const name = release.name ? release.name : release.tag_name;
                span.innerHTML += '<h3>' + name + '</h3>';
                span.innerHTML += '<p><em>Released: ' + release.published_at + '</em></p>';
                /*for (const line of release.body.split('\n')) {
                    span.innerHTML += '<p>' + line + '</p>';
                }*/
                span.innerHTML += '<a href="/" class="book-btn" style="margin-right: 1rem;">Windows</a>';
                span.innerHTML += '<a href="/" class="book-btn" style="margin-right: 1rem;">Linux</a>';
                span.innerHTML += '<a href="/" class="book-btn" style="margin-right: 1rem;">Mac OSX</a>';
                span.innerHTML += '<a href="/" class="book-btn">Source code</a>';
                target.parentNode.insertBefore(span, target.nextSibling);
            }
        });
}
