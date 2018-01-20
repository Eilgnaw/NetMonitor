#!/usr/bin/python
#_*_coding:utf-8_*_
import sys
reload(sys)
sys.setdefaultencoding('utf8')
import datetime
import time
import requests
import json
import urlparse
import os.path
import urllib,urllib2

def sendmsg(newip):
    #修改为自己的钉钉机器人推送地址
    url = "https://oapi.dingtalk.com/robot/send?access_token=fc11b558bfde65ad194efe86e0dce1cdf96a1520bf3d2437a00388cdcf506b78"
    content = "本地服务器新 IP 地址为" + newip
    actionbaseURL = "http://"+ newip +":3000"#因为我在本地服务器上开了3000端口的服务,顺手点一下可以验证是否正确
    amap = "amap"
    downloadBaseURL = "http://" +newip+":3000"
    HEADERS = {
      "Content-Type": "application/json ;charset=utf-8 "
    }

    String_textMsg ={\
    "msgtype": "actionCard", \
    "actionCard":{\
    "title": "本地服务器更改后地址",\
    "text":content,\
    "hideAvatar":"1",\
    "btnOrientation": "0",\
    "btns":[{\
    "title":"别点我",\
    "actionURL": downloadBaseURL },
     {\
    "title":"也别点我",\
    "actionURL": actionbaseURL}
    ] }}
    String_textMsg1 = json.dumps(String_textMsg)
    res = requests.post(url, data=String_textMsg1, headers=HEADERS)
    return res.text

if __name__=="__main__":
    sendmsg(sys.argv[1])