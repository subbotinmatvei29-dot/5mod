cmd_/home/matvei/kernel/modules.order := {   echo /home/matvei/kernel/hello.ko; :; } | awk '!x[$$0]++' - > /home/matvei/kernel/modules.order
