import requests ;

payload = {'key': ''}
url = 'http://127.0.0.1:8000/api/battle/end'
r = requests.post(url, data=dumps(payload))
print(r.status_code)
print(r.text)
