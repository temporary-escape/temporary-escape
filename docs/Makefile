theme = themes/hugo-book

$(theme):
	git clone https://github.com/alex-shpak/hugo-book.git themes/hugo-book

generate:
	python3 gen-download.py --endpoint "https://${AWS_ENDPOINT_URL}" --bucket ${AWS_BUCKET_NAME} > ./content/docs/download.md

hugo: $(theme) generate
	hugo
	@echo "Done"

default: hugo

.PHONY: generate hugo default
