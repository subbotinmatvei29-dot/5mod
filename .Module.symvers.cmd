cmd_/home/matvei/kernel/Module.symvers := sed 's/\.ko$$/\.o/' /home/matvei/kernel/modules.order | scripts/mod/modpost -m -a  -o /home/matvei/kernel/Module.symvers -e -i Module.symvers   -T -
