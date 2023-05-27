# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "Temporary Escape"
copyright = "2023, Matus Novak"
author = "Matus Novak"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["myst_parser", "sphinx_design"]

templates_path = ["_templates"]
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

css_variables = {
    # Taken from: https://github.com/pradyunsg/furo/blob/c682d5d3502f3fa713c909eebbf9f3afa0f469d9/src/furo/assets/styles/variables/_colors.scss
    "color-problematic": "#ee5151",

    # Base Colors
    "color-foreground-primary": "#ffffffcc", # for main text and headings
    "color-foreground-secondary": "#9ca0a5", # for secondary text
    "color-foreground-muted": "#81868d", # for muted text
    "color-foreground-border": "#666666", # for content borders

    "color-background-primary": "#131416", # for content
    "color-background-secondary": "#1a1c1e", # for navigation + ToC
    "color-background-hover": "#1e2124ff", # for navigation-item hover
    "color-background-hover--transparent": "#1e212400",
    "color-background-border": "#303335", # for UI borders
    "color-background-item": "#444", # for "background" items (eg: copybutton)

    # Announcements
    "color-announcement-background": "#000000dd",
    "color-announcement-text": "#eeebee",

    # Brand colors
    "color-brand-primary": "#fcdb0c",
    "color-brand-content": "#fcdb0c",

    # Highlighted text (search)
    "color-highlighted-background": "#083563",

    # GUI Labels
    "color-guilabel-background": "#08356380",
    "color-guilabel-border": "#13395f80",

    # API documentation
    "color-api-keyword": "var(--color-foreground-secondary)",
    "color-highlight-on-target": "#333300",

    # Admonitions
    "color-admonition-background": "#18181a",

    # Cards
    "color-card-border": "var(--color-background-secondary)",
    "color-card-background": "#18181a",
    "color-card-marginals-background": "var(--color-background-hover)",
}

html_theme = "furo"

html_static_path = ["_static"]

html_logo = "../../logo/logo-docs.png"

html_theme_options = {
    "dark_css_variables": css_variables,
    "light_css_variables": css_variables,
    "sidebar_hide_name": True,
    "announcement": "This is a work in progress game!",
    "footer_icons": [
        {
            "name": "GitHub",
            "url": "https://github.com/temporary-escape/temporary-escape",
            "html": """
                <svg stroke="currentColor" fill="currentColor" stroke-width="0" viewBox="0 0 16 16">
                    <path fill-rule="evenodd" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"></path>
                </svg>
            """,
            "class": "",
        },
    ],
    "source_edit_link": "https://github.com/temporary-escape/temporary-escape/tree/master/docs/content/{filename}",
}

html_css_files = [
    "css/custom.css",
]

pygments_style = "sphinx"
pygments_dark_style = "monokai"
