# Matchmaker

## Run locally (TODO)

```bash
# Create Python virtual environment
python3 -m venv ./venv

# Activate the virtual environment
source ./venv/bin/activate

# Install all dependencies
python3 -m pip install .

# Run the app locally on http://127.0.0.1:8000
python3 -m uvicorn matchmaker.app:app --port 8000 --log-config=log_conf.yaml
```


## Helm install (TODO)

```bash
helm upgrade --install matchmaker ./chart/ --values ~/.temporary-escape-chart-values.yaml --namespace matchmaker --create-namespace
```

## Create local certs (TODO)

```bash
openssl req \
    -x509 \
    -nodes \
    -days 3650 \
    -subj "/C=US/ST=/L=/O=/OU=/CN=matchmaker.lan/emailAddress=" \
    -newkey rsa:2048 \
    -keyout "${SCRIPT_DIR}/certs/server.key" \
    -out "${SCRIPT_DIR}/certs/server.crt"
```

## Traefik with local certs (TODO)

```yaml
tls:
  stores:
    default:
      defaultCertificate:
        certFile: /tmp/matchmaker/certs/server.crt
        keyFile: /tmp/matchmaker/certs/server.key
  certificates:
    - certFile: /tmp/matchmaker/certs/server.crt
      keyFile: /tmp/matchmaker/certs/server.key
```
