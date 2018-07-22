1.    << 追加输出  < 覆盖输出

2.    所有变量都被看做字符串来存储，不需要事先为它们声明，在变量前加 $ 符号就可以来访问它们的内容。可以用read命令让用户输入，输入完，按回车即read命令结束。

      ```shell
          $ read salutation

          Wie geht's?

          $ echo salutation

          Wie geht's?
      ```

3.    环境变量 ：

          $HOME 当前用户家目录

          $PATH 以冒号分割的用来搜索命令的目录列表

          $0 shell脚本的名字

          $# 传递给脚本的参数个数

          $$ shell脚本的进程号，脚本程序通常会用它来生成一个唯一的临时文件

4.    条件： test 或 [  ] ，判断文件fred.c是否存在

      ```shell
          if test -f fred.c
          then 
          ...
          fi

          if [ -f fred.c ]; then 
          ...
          fi

          #elif

          #!/bin/sh

          echo "Is it morning ? Please answer yes or no "
          read timeofday

          if [ "$timeofday" = "yes" ]; then
              echo "Good morning"
          elif [ "$timeofday" = "no" ]; then
              echo "Good afternoon"
          else
              echo "Sorry , $timeofday not recognized. Enter yes or no"
              exit 1
          fi

          exit 0

      ```

5.    for 语句

      ```shell
          #!/bin/sh

          for foo in bar fud 43
          do
            echo $foo
          done
          exit 0

          #!/bin/sh

          for file in $(ls f*.sh); do
             lpr $file
          done
          exit 0
      ```

6.    while 语句

      ```shell
          #!/bin/sh

          echo "Enter password"
          read trythis

          while [ "$trythis" != "secret" ]; do
             echo "Sorry, try again"
             read trythis
          done
          exit 0
      ```

7.    until 语句

      ```shell
          #!/bin/sh

          until who | grep "$1" > dev/null
          do
             sleep 60
          done 

          #now ring the bell and announce the expected user.

          echo -e '\a'
          echo "******* $1 has just logged in ***********"

          exit 0
      ```

8.    case 语句

      ```shell
        #!/bin/sh

        echo "Is it morning? Please answer yes or no"
        read timeofday
         
        case "$timeofday" in
             yes) echo "Good Morning";;
             no ) echo "Good Afternoon";;
             y  ) echo "Good Morning";;
             n  ) echo "Good Afternoon";;
      *  ) echo "Sorry, answer not recognized";;
      ```
     esac
     exit 0
    
     #合并模式
     case "$timeofday" in
          yes | y | Yes | YES ) echo "Good Morning";;
          n*  | N* )            echo "Good Afternoon";;
          * )                   echo "Sorry, answer not recognized";;
     esac
    
     exit 0


     ```

9.   if 条件太多可以考虑用 AND 列表和 OR 列表来合并条件项

     statement1 && statement2 && statement3 && ...

     statement1 || statement2 || statement3 || ...

     ```shell
     #!/bin/sh

     rm -f file_one

     if[ -f file_one ] || echo "Hello" || echo " there"
     then 
         echo "in if"
     else
         echo "in else"
     fi
     exit 0
     ```

10.   函数

     - 函数的定义：

      ​             function_name () {

      ​                 statements

      ​            }

     -  从函数返回一个值

     ```shell
        #!/bin/sh

        yes_or_no(){
          
          echo "Is your name $*"
          while true
          do 
             echo -n "Enter yes or no: "
             read x
             case "$x" in
               y | yes ) return 0;;
               n | no  ) return 1;;
               * )       echo "Answer yes or no"
               esac
           done
        }
     ```

     11.   内置命令：
          - break;    break 只跳出一个循环

          - :       true的一个别名

          - continue

          - .   用于在当前shell中执行命令

          - echo     echo -n (换行)

          - eval 一个额外的 $

          - exec  将当前shell替换为一个不同的程序

          - exit n (0代表成功，126文件不可执行,127命令未找到,128及以上出现一个信号)

          - export 导出变量  导出的变量在子shell中也可使用

          - expr 将参数当做一个表达式来求值 x = \`expr &x + 1\`

          - set 为shell设置参数变量

          - shift 将所有的参数变量左移一个位置

          - trap 用于指定在接收到信号后将要采取的行动 { HUP(1)挂起  INT(2)中断  QUIT(3)退出  ABRT(6)中止  ALRM(14)报警   TERM(15)终止 }

          - unset 从环境中删除变量或函数

          - find 用于搜索文件的命令

          - grep 用户搜索字符串的命令

          - $(command) 将命令的输出放到一个变量中去

          - 参数拓展

            ```shell
            #!/bin/sh
             for i in 1 2
             do 
                my_secret_process ${i}_tmp
             done
             exit 0
            ```

          - $((...))算术拓展  

            ```shell
            #!/bin/sh
            x=0
            while [ "$x" -ne 10 ]; do
                  echo $x
                  x=$(($x +1 ))
            done
            exit 0
            ```

​                   注意：两对圆括号用于算术替换，一对圆括号用于命令的执行和获取输出。            