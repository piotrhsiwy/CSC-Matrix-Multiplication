if [[ -z "${BINARY_TO_TEST}" ]]; then
    echo "you need to set BINARY_TO_TEST"
    exit 0
fi



for dir in */ ; do
    for alg in $(seq 0 5); do
        ../main -V${alg} -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt > $dir/log.txt
        cmp $dir/C.txt $dir/expected.txt
        echo "$dir, alg:-V${alg}: last cmp returned: $?"
    done
done
