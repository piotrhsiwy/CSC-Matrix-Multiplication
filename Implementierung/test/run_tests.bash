if [[ -z "${BINARY_TO_TEST}" ]]; then
    echo "you need to set BINARY_TO_TEST"
    exit 0
fi



for dir in */ ; do
    ../main -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt > $dir/log.txt
    cmp $dir/C.txt $dir/expected.txt
    echo "$dir: last cmp returned: $?"
done
