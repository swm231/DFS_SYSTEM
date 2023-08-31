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

#define _public "<div style=\"text-align: center\">\n\
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
            async function getServer_file() {\n\
                try {\n\
                    const response = await fetch('/get_public_server');\n\
                    if (response.ok) {\n\
                        const fileInput = document.getElementById('upload');\n\
                        const file = fileInput.files[0];\n\
                        return { response, file };\n\
                    }\n\
                }\n\
                catch (error) {\n\
                    console.error(\"Error in UpLoad:\", error);\n\
                }\n\
            }\n\
            async function getServer() {\n\
                try {\n\
                    const response = await fetch('/get_public_server');\n\
                    if (response.ok) {\n\
                        return response;\n\
                    }\n\
                }\n\
                catch (error) {\n\
                    console.error(\"Error in UpLoad:\", error);\n\
                }\n\
            }\n\
            async function UpLoad(){\n\
                const { response, file } = await getServer_file();\n\
                const uploadServer = await response.text() + \"/public/upload/\" + file.name;\n\
                const formData = new FormData();\n\
                formData.append('upload', file);\n\
                await fetch(uploadServer, {\n\
                    method: 'POST',\n\
                    body: formData\n\
                });\n\
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
                const response = await getServer();\n\
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
            async function getServer_file() {\n\
                try {\n\
                    const response = await fetch('/get_private_server');\n\
                    if (response.ok) {\n\
                        const fileInput = document.getElementById('upload');\n\
                        const file = fileInput.files[0];\n\
                        return { response, file };\n\
                    }\n\
                }\n\
                catch (error) {\n\
                    console.error(\"Error in UpLoad:\", error);\n\
                }\n\
            }\n\
            async function getServer() {\n\
                try {\n\
                    const response = await fetch('/get_private_server');\n\
                    if (response.ok) {\n\
                        return response;\n\
                    }\n\
                }\n\
                catch (error) {\n\
                    console.error(\"Error in UpLoad:\", error);\n\
                }\n\
            }\n\
            async function UpLoad(){\n\
                const { response, file } = await getServer_file();\n\
                const uploadServer = await response.text() + \"/private/upload/\" + file.name;\n\
                const formData = new FormData();\n\
                formData.append('upload', file);\n\
                const xhr = new XMLHttpRequest();\n\
                xhr.upload.addEventListener(\"progress\", (event) => {\n\
                    if (event.lengthComputable) {\n\
                        const percentComplete = (event.loaded / event.total) * 100;\n\
                        console.log(`Upload Progress: ${percentComplete.toFixed(2)}%`);\n\
                    }\n\
                });\n\
                xhr.open(\"POST\", uploadServer, true);\n\
                xhr.onload = () => {\n\
                    if (xhr.status === 200) {\n\
                        console.log(\"Upload complete!\");\n\
                    } else {\n\
                        console.error(\"Upload failed\");\n\
                    }\n\
                };\n\
                xhr.send(formData);\n\
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
                const response = await getServer();\n\
                const uploadServer = await response.text() + \"/private/delete/\" + filename;\n\
                await fetch(uploadServer, {\n\
                    method: 'DELETE'\n\
                });\n\
                // location.reload();\n\
            }\n\
        </script>"
#define _htmlend "</body></html>"
