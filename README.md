   # 目录：
   ## 一、项目总览
   ### 1.1 项目链接
   ### 1.2.框架介绍
   ## 二、bsp层
   ### 2.1.蜂鸣器\buzzer
   ### 2.2.彩灯\led
   ### 2.3.串口\UART
   ### 2.4.can通信
   ### 2.5.陀螺仪\bmi088
   ## 三、app层
   ### 3.1.PID
   ### 3.2.ins角解算
   ### 3.3.任务\freeRTOS
   3.3.1.motor_ctrl
   
   3.3.2.community
   
   3.3.3.task_status
   ## 四、components层
   ### 4.1.电机\motor
   4.1.1.dji电机
   
   4.1.2.dm电机
   ### 4.2.遥控器\remote


# --------------------------正文--------------------------


# 1.1.项目链接
GitHub: [GitHub - xiaoT487/em2: the second exam · GitHub](https://github.com/xiaoT487/em2/tree/main)
**1.克隆代码：** ```shell git clone  https://github.com/xiaoT487/em2``` 复制代码到终端
**2.点击`.ioc`生成代码**
**3.删除`freeRTOS.c`里的默认生成`Start`函数**



   
   # 1.2.框架介绍
   >## 所有自建文件均在`user_config`中
   
   >### 1.`bsp`层：
   框架中的底层，主要是一些单片机的基本功能的封装，**包含：**
   - 蜂鸣器
   - led
   - 串口
   - can通信
   - 陀螺仪

>### 2.`app`层：
框架中的应用层，通常是一些需要算法或者任务功能实现的代码，**包含：**
- 任务
- pid
- ins角解析

>### 3,`components`层：
外接设备层，需要通过各种方式去外接一些设备，**包含：**
- 电机
- 遥控器