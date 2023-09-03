#pragma once

#define _400 "<div style=\"text-align: center;\">    <p id=\"400\">        400 无法解析请求    </p></div></body></html>"
#define _403 "<div style=\"text-align: center;\">    <p id=\"403\">        403 禁止访问    </p></div></body></html>"
#define _404 "<div style=\"text-align: center;\">    <p id=\"404\">        404 没有找到该页面    </p></div></body></html>"
#define _head "<body>    <style>      </style>            <div class=\"header\">        <div class=\"Title\">          <ul>            <li><a class=\"navigation\" href=\"/\">首页</a></li>            <li><a class=\"navigation\" href=\"/public\">公共空间</a></li>            <li><a class=\"navigation\" href=\"/private\">个人空间</a></li>          </ul>        </div>              <div class=\"User\">          <ul>            <li><a class=\"navigation\" href=\"/login\">登录</a></li>            <li><a class=\"navigation\" href=\"/register\">注册</a></li>          </ul>        </div>      </div>"
#define _head_ "<body>    <style>      </style>            <div class=\"header\">        <div class=\"Title\">          <ul>            <li><a class=\"navigation\" href=\"/\">首页</a></li>            <li><a class=\"navigation\" href=\"/public\">公共空间</a></li>            <li><a class=\"navigation\" href=\"/private\">个人空间</a></li>          </ul>        </div>"
#define _index "<div style=\"text-align: center;\">    <p id=\"welcome\">        欢迎来到个人文件管理系统    </p></div></body></html>"

#define _login "    <div style=\"text-align: center\">\n\
        <h1>登录</h1>\n\
        <form action=\"login\" method=\"post\">\n\
            <div style=\"text-align: center\">\n\
                <input type=\"text\" name=\"username\" placeholder=\"用户名\" required=\"required\">\n\
            </div> <br/>\n\
            <div style=\"text-align: center\">\n\
                <input type=\"password\" name=\"password\" placeholder=\"用户密码\" required=\"required\">\n\
            </div> <br/>\n\
            <div style=\"text-align: center\"><button type=\"submit\">确认</button></div>\n\
        </form>\n\
    </div></body></html>"

#define _namerr "<div style=\"text-align: center;\">    <p id=\"welcome\">        用户名错误!    </p></div></body></html>"

#define _pwderr "<div style=\"text-align: center;\">    <p id=\"welcome\">        密码错误!    </p></div></body></html>"

#define _private "<div style=\"text-align: center\">\n\
    <br>        私人空间    <br/>\n\
</div><div style=\"text-align: center\">\n\
    <br>\n\
    <div style=\"width:300px; text-align: left; margin: auto;\">选择文件上传：</div>\n\
    <br/>\n\
    <form id=\"uploadfile\" action=\"\" method=\"post\" enctype=\"multipart/form-data\" style=\"text-align: center;\">\n\
        <input type=\"file\" id=\"upload\" name=\"upload\" style=\"border:1px solid;\"/>\n\
        <input onclick=\"requestUploadServer();\" value=\"请求 Upload-Server\">\n\
    </form>\n\
</div><div style=\"text-align: center\">\n\
    <br>\n\
    <div style=\"width:300px; text-align: left; margin: auto;\">文件列表：</div>\n\
    <br/>\n\
    <table border=\"1px\" style=\"width:600px;text-align: center; margin: auto;table-layout:fixed;\">\n\
        <thead>\n\
            <td style=\"text-align: center;\">文件名</td> <td></td> <td></td>\n\
        </thead>"

#define _public "        <div id=\"div1\" style=\"width: 100%; height: 100%; display: none;text-align: center; background-color: #8b8b8b; opacity: 0.5;\" ></div> \n\
        <div id=\"div2\" style=\"width: 200px; height: 30px; text-align: center; display: none; padding: 30px 30px; font-size: large; opacity: 1; background-color:#ffffff; border-radius: 20px; border: 1px solid #5e5c5c4b;\">正在上传文件...</div> \n\
        <div style=\"text-align: center\">\n\
    <br>        公共空间    <br/>\n\
</div><div style=\"text-align: center\">\n\
    <br>\n\
    <div style=\"width:300px; text-align: left; margin: auto;\">选择文件上传：</div>\n\
    <br/>\n\
    <form id=\"uploadfile\" action=\"\" method=\"post\" enctype=\"multipart/form-data\" style=\"text-align: center;\">\n\
        <input type=\"file\" id=\"upload\" name=\"upload\" style=\"border:1px solid;\"/>\n\
        <input id=\"uploadButton\" type=\"button\" onclick=\"UpLoad();\" value=\"上传\">\n\
    </form>\n\
</div><div style=\"text-align: center\">\n\
    <br>\n\
    <div style=\"width:300px; text-align: left; margin: auto;\">文件列表：</div>\n\
    <br/>\n\
    <table border=\"1px\" style=\"width:600px;text-align: center; margin: auto;table-layout:fixed;\">\n\
        <thead>\n\
            <td style=\"text-align: center;\">文件名</td> <td></td> <td></td>\n\
        </thead>"

#define _register "    <div style=\"text-align: center\">\n\
        <h1>注册</h1>\n\
        <form action=\"register\" method=\"post\">\n\
            <div style=\"text-align: center\">\n\
                <input type=\"text\" name=\"username\" placeholder=\"用户名\" required=\"required\">\n\
            </div> <br/>\n\
            <div style=\"text-align: center\">\n\
                <input type=\"password\" name=\"password\" placeholder=\"用户密码\" required=\"required\">\n\
            </div> <br/>\n\
            <div style=\"text-align: center\"><button type=\"submit\">确认</button></div>\n\
        </form>\n\
    </div></body></html>"
    
#define _title "<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
    <meta charset=\"UTF-8\">\n\
    <title>个人文件管理系统</title>\n\
    <style>\n\
        .header{\n\
            margin-right: 12px;\n\
            display: flex;\n\
            justify-content: space-between;\n\
        }\n\
        .header li{\n\
            list-style: none;\n\
            display: inline;\n\
            padding: 12px;\n\
            font-size: 25px;\n\
        }\n\
        .header a{\n\
            text-decoration: none;\n\
            color: black;\n\
        }\n\
        .Title{\n\
            text-align: left;\n\
        }\n\
        .User{\n\
            text-align: right;\n\
        }\n\
        #welcome{\n\
            font-size: 30px;\n\
        }\n\
        .navigation {\n\
            text-decoration: none;\n\
            color: #000;\n\
            padding: 5px 10px;\n\
        }\n\
        .load{\n\
            text-decoration: none;\n\
            color: black;\n\
            font-size: 20px;\n\
        }\n\
    </style>\n\
</head>"

#define _welcome  "<div style=\"text-align: center;\">    <p id=\"welcome\">        登录成功!    </p></div></body></html>"
#define _listend "</table></div>"
#define _publicjs "<script type=\"text/javascript\">\n\
            function confirmDelete() {\n\
                return confirm('确认删除该文件吗？');\n\
            }\n\
            function uploadWin(){\n\
                var vDiv = document.getElementById('div1');\n\
                vDiv.style.display = 'block';\n\
                vDiv.style.zIndex = 10;\n\
                vDiv.style.position = \"absolute\";\n\
                var vDiv2 = document.getElementById('div2');\n\
                vDiv2.style.display = 'block';\n\
                vDiv2.style.zIndex = 11;\n\
                vDiv2.style.position = \"fixed\";\n\
                vDiv2.style.top = \"20%\";\n\
                vDiv2.style.left = \"50%\";\n\
                vDiv2.style.marginLeft = \"-111px\";\n\
                vDiv2.style.marginTop = \"-20px\";\n\
            }\n\
            async function postServer_file() {\n\
                const response = await fetch('/get_public_server',{\n\
                    method: 'POST' \n\
                });\n\
                if (response.ok) {\n\
                    const fileInput = document.getElementById('upload');\n\
                    const file = fileInput.files[0];\n\
                    return { response, file };\n\
                }\n\
            }\n\
            async function postServer() {\n\
                const response = await fetch('/get_public_server', {\n\
                    method: 'POST' \n\
                });\n\
                if (response.ok) {\n\
                    return response;\n\
                }\n\
            }\n\
            async function getServer() {\n\
                const response = await fetch('/get_public_server', {\n\
                    method: 'GET' \n\
                });\n\
                if (response.ok) {\n\
                    return response;\n\
                }\n\
            }\n\
            function closeWin() {\n\
                var vDiv1 = document.getElementById('div1');\n\
                var vDiv2 = document.getElementById('div2');\n\
                vDiv1.style.display = 'none';\n\
                vDiv2.style.display = 'none';\n\
            }\n\
            async function UpLoad(){\n\
                const { response, file } = await postServer_file();\n\
                const uploadServer = await response.text() + \"/public/upload/\" + file.name;\n\
                const formData = new FormData();\n\
                formData.append('upload', file);\n\
                await fetch(uploadServer, {\n\
                    method: 'POST',\n\
                    body: formData\n\
                });\n\
                closeWin();\n\
            }\n\
            async function DownLoad(filename){\n\
                const response = await getServer();\n\
                const uploadServer = await response.text() + \"/public/download/\" + filename;\n\
                const a = document.createElement('a');\n\
                a.style.display = 'none';\n\
                document.body.appendChild(a);\n\
                a.href = uploadServer;\n\
                a.download = filename;\n\
                a.click();\n\
                document.body.removeChild(a);\n\
            }\n\
            async function Delete(filename){\n\
                if(!confirmDelete())\n\
                    return;\n\
                const response = await postServer();\n\
                const uploadServer = await response.text() + \"/public/delete/\" + filename;\n\
                await fetch(uploadServer, {\n\
                    method: 'DELETE'\n\
                });\n\
                // location.reload();\n\
            }\n\
        </script>"

#define _privatejs "<script type=\"text/javascript\">\n\
            function confirmDelete() {\n\
                return confirm('确认删除该文件吗？');\n\
            }\n\
            function uploadWin(){\n\
                var vDiv = document.getElementById('div1');\n\
                vDiv.style.display = 'block';\n\
                vDiv.style.zIndex = 10;\n\
                vDiv.style.position = \"absolute\";\n\
                var vDiv2 = document.getElementById('div2');\n\
                vDiv2.style.display = 'block';\n\
                vDiv2.style.zIndex = 11;\n\
                vDiv2.style.position = \"fixed\";\n\
                vDiv2.style.top = \"20%\";\n\
                vDiv2.style.left = \"50%\";\n\
                vDiv2.style.marginLeft = \"-111px\";\n\
                vDiv2.style.marginTop = \"-20px\";\n\
            }\n\
            async function postServer_file() {\n\
                const response = await fetch('/get_private_server',{\n\
                    method: 'POST' \n\
                });\n\
                if (response.ok) {\n\
                    const fileInput = document.getElementById('upload');\n\
                    const file = fileInput.files[0];\n\
                    return { response, file };\n\
                }\n\
            }\n\
            async function postServer() {\n\
                const response = await fetch('/get_private_server',{\n\
                    method: 'POST' \n\
                });\n\
                if (response.ok) {\n\
                    return response;\n\
                }\n\
            }\n\
            async function getServer() {\n\
                const response = await fetch('/get_private_server',{\n\
                    method: 'GET' \n\
                });\n\
                if (response.ok) {\n\
                    return response;\n\
                }\n\
            }\n\
            function closeWin() {\n\
                var vDiv1 = document.getElementById('div1');\n\
                var vDiv2 = document.getElementById('div2');\n\
                vDiv1.style.display = 'none';\n\
                vDiv2.style.display = 'none';\n\
            }\n\
            async function UpLoad(){\n\
                const { response, file } = await postServer_file();\n\
                const uploadServer = await response.text() + \"/private/upload/\" + file.name;\n\
                const formData = new FormData();\n\
                formData.append('upload', file);\n\
                await fetch(uploadServer, {\n\
                    method: 'POST',\n\
                    body: formData\n\
                });\n\
                closeWin();\n\
            }\n\
            async function DownLoad(filename){\n\
                const response = await getServer();\n\
                const uploadServer = await response.text() + \"/private/download/\" + filename;\n\
                const a = document.createElement('a');\n\
                a.style.display = 'none';\n\
                document.body.appendChild(a);\n\
                a.href = uploadServer;\n\
                a.download = filename;\n\
                a.click();\n\
                document.body.removeChild(a);\n\
            }\n\
            async function Delete(filename){\n\
                if(!confirmDelete())\n\
                    return;\n\
                const response = await postServer();\n\
                const uploadServer = await response.text() + \"/private/delete/\" + filename;\n\
                await fetch(uploadServer, {\n\
                    method: 'DELETE'\n\
                });\n\
                // location.reload();\n\
            }\n\
        </script>"
#define _htmlend "</body></html>"
