cmd_/home/matvei/kernel/2/modules.order := {   echo /home/matvei/kernel/2/userspace.ko; :; } | awk '!x[$$0]++' - > /home/matvei/kernel/2/modules.order
