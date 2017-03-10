for d in /dev/* ; do
    echo $(echo $d && stat -c '%t %T' $d)
done > dev_tty_data.txt
