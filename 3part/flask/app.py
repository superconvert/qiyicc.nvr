from flask import Flask, request

app = Flask(__name__)

@app.route('/')
def index():
    return "hello nvr flask"

@app.route('/test/<int:user_id>')
def test(user_id):
    return user_id

@app.route('/api/v1/notice', methods=["POST", "GET"])
def notice():
    #request.form['username']
    #request.get_json()
    return "hello nvr api flask" + request.args.get('cmd') 

@app.route('/api/v1/information')
def information():
    return "hello nvr api flask"

@app.route('/api/v1/vod/upload')
def vod_upload():
    return "hello nvr api flask"

@app.route('/api/v1/vod/recorder')
def vod_recorder():
    return "hello nvr api flask"

if __name__ == '__main__':
    app.run()
