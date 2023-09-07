# DFS_SYSTEM

分布式文件存储系统

![public](https://github.com/swm231/DFS_SYSTEM/blob/master/img/public.png)

服务器分为两类节点，一类为 tracker 跟踪节点，一类为 storage 存储节点。

**跟踪节点**负责 用户http、登录、注册等请求

**存储节点**负责 用户上传、下载、删除文件请求



## 使用演示

[前往Bilibili观看](https://www.bilibili.com/video/BV1Hu411P7pd/)



## 功能 与 技术支持

- `tracker`节点
  - `http`通信
    - #define存储 html 代码，发送时减少上下文切换次数
    - 用户登录发送 cookie 记录登录信息，避免重复登陆
    - 用户登录后使用小根堆计时器，长时间未操作服务器主动断开 TCP 连接
    - 文件操作先使用一致性哈希找到相应分组，再在分组找合适节点给客户端操作
    - 支持使用 nginx 添加多个 tracker 节点
  - `storage`信息
    - 写优先读写锁保证线程安全
    - 增加节点、公共空间更新调用写锁
    - 个人空间文件操作不使用写锁，同时保证同组信息正常同步
- `storage` 节点
  - `http`通信
    - 文件发送使用 sendfile 零拷贝技术
    - 使用 MySQL 保证用户文件逻辑上一致
    - 市容日志系统保证用户文件物理上一致
  - `tracker`信息
    - 追踪节点信息在文件 storage.conf 文件中提前配置
    - 可同时配置多台 tracker 节点
- 数据一致性
  - `rollback`日志
    - 回滚日志 & 内存日志
    - 防止用户意外断连，用于数据回滚

  - `syn`日志
    - 同步日志 & 磁盘日志
    - 防止节点意外断连，用于数据恢复

  - `ring`日志
    - 循环日志 & 磁盘日志
    - 日志文件大小、每条日志大小固定，采用双链表维护
    - 保证 `rollbacklog` `synlog` 顺利完成

- 通信
  - ET模式的 `Epoll` 通信
    - epoll_data_t 使用 void* 指针指向 BaseNode 基类
    - 接收器、client、tracker、storage均由 BaseNode 派生
    - 相应事件由多态特性分别进行处理
  - 节点 `socket` 通信
    - 自定义通信结构，可发送同步信息、文件等
    - 首部大写字母表示首次通信、小写字母表示被动通信
- 其他
  - buffer 缓存区，缓存 接收 或 发送 消息
  - log 同步异步日志，生产者消费者模式，4种日志等级
  - 线程池、连接池，防止频繁创建线程或建立 MySQL 连接



## 结构

### 整体框架

![dfs_system](https://github.com/swm231/DFS_SYSTEM/blob/master/img/dfs_system.png)

### 日志系统

#### 流程图

![logflow](https://github.com/swm231/DFS_SYSTEM/blob/master/img/logflow.png)



## 使用说明

tracker 和 storage 的配置文件与 CMakeLists.txt 同级，其中

- tracker节点

  - `cookieOut` 用户 cookie 有效时间

  - `timeOut` 用户未操作断开连接时间

- storage节点

  - `tracker_server` tracker 节点 ip 地址及工作端口
  - `data_path` 文件保存地址
  - `data_capacity` 文件存储容量上限

- 公共

  - `bind_addr` socket 绑定地址

  - `http_port` http 监听端口

  - `task_port` 节点工作端口
  - `mysql` 配置
    - `mysql_host` 用户信息数据库连接主机
    - `mysql_user` 用户信息数据库连接用户名
    - `mysql_pwd` 用户信息数据库连接密码
    - `mysql_database` 用户信息

数据库在使用之前，应当在 `mysql` 对应库中创建表：

```sql
CREATE TABLE `user` (
  `username` char(50) NOT NULL,
  `password` char(50) NOT NULL,
  `cookie` varchar(16) DEFAULT NULL,
  -- 根据用户名查询密码，使用主键索引
  PRIMARY KEY(username),
  -- 根据 cookie 查询用户名，用户名为主键，使用索引覆盖
  UNIQUE KEY `cookie_index` (`cookie`)
);
```

建立 public 及 private 文件索引

```sql
CREATE TABLE `public` (
  `filename` varchar(50) NOT NULL,
  -- 这里经常会进行全表扫描，普通索引减少磁盘IO
  INDEX `idx_filename` (`filename`)
);
CREATE TABLE `private` (
  `username` char(16) NOT NULL,
  `filename` varchar(50) NOT NULL,
  -- 查找 username 相同的 filename，使用联合索引
  INDEX `idx_username_filename` (`username`, `filename`)
);
```

## 运行

本工程使用 `cmake`，已使用`shell`脚本

tracker 端：(在 tracker 目录中)

```shell
./config.sh
./start.sh
```

storage 端：(在 storage目录中)

```shell
./config.sh
./start.sh
```

停止服务

```
./stop.sh
```



## TODO LIST

- 更高效的 `storage` 自动同步方式
  - 新的 storage 节点直接全盘同步
  - 临时退出 3 天的 storage 节点
    - 数据库记录 3 天之内用户操作及操作时间
    - 上线后对数据库中 离线 时操作的文件名进行哈希
    - 对操作进行分类，上传表示 1 删除表示 -1
    - 对操作值求 前缀和 后，1表示要同步的文件，-1表示要删除的文件，0表示不需要操作的文件
- 缓存机制
  - 对于热点小文件，设置分级缓存
  - 使用有门槛的 LRU 维护热点数据
  - 对磁盘日志进行缓存
- 水平扩展
  - 重新计算 哈希环 达到动态水平扩展
- ...















