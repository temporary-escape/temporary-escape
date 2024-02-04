# Matchmaker

## Run locally

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
