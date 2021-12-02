
lsof -i:$bbr_port -t | xargs kill -9
lsof -i:$loss_port -t | xargs kill -9

