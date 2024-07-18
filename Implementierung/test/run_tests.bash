if [[ -z "${BINARY_TO_TEST}" ]]; then
    echo "you need to set BINARY_TO_TEST"
    exit 0
fi

if [ ! -s "${FILENAME}" ]; then
    echo "File is empty"
else
    echo "File is not empty"

fi

for dir in invalid/* ; do
    ${BINARY_TO_TEST} -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt > $dir/log.txt
    if [ ! -s "$dir/log.txt" ]; then
        echo "- $dir/log.txt is empty! (ret $?) Here is the explanation:"
        cat $dir/explanation.txt
    else
        echo "+ $dir/log.txt is not empty! (ret $?) Comparing to expected.txt..."
        cmp $dir/log.txt $dir/expected.txt
        if [ $? = 1 ]; then
            echo "- Not the same, here is diff output: (ret $?)"
            diff $dir/log.txt $dir/expected.txt
        else
            echo "+ They are the same picture! (ret $?)"
        fi
    fi
done

for dir in valid/* ; do
    for alg in $(seq 0 5); do
        ${BINARY_TO_TEST} -V${alg} -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt > $dir/log.txt
        cmp $dir/C.txt $dir/expected.txt
        echo "$dir, alg:-V${alg}: last cmp returned: $?"
    done
done