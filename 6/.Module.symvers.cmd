cmd_/home/matvei/kernel/6/Module.symvers := sed 's/\.ko$$/\.o/' /home/matvei/kernel/6/modules.order | scripts/mod/modpost -m -a  -o /home/matvei/kernel/6/Module.symvers -e -i Module.symvers   -T -
