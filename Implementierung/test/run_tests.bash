if [[ -z "${BINARY_TO_TEST}" ]]; then
    echo "you need to set BINARY_TO_TEST"
    exit 0
fi

if [ ! -s "${FILENAME}" ]; then
    echo "File is empty"
else
    echo "File is not empty"

fi

clear
for dir in invalid/* ; do
    echo "----------------------------- Testcase $dir -----------------------------"
    ${BINARY_TO_TEST} -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt 2>$dir/log.txt
    if [ $? = 1 ]; then
        # echo "+ return value is correctly EXIT_FAILURE (ret $?). Checking stderr output..."
        if [ ! -s "$dir/log.txt" ]; then
            echo "BAD $dir/log.txt is empty! (ret $?) Here is the explanation:"
            cat $dir/README
        else
            # echo "+ $dir/log.txt is not empty! (ret $?) Comparing to expected.txt..."
            cmp $dir/log.txt $dir/expected.txt
            if [ $? = 1 ]; then
                echo "BAD Not the same, here is diff output: (ret $?)"
                diff $dir/log.txt $dir/expected.txt
            # else
                # echo "+ They are the same picture! (ret $?)"
            fi
        fi
    else
        echo "BAD return value is incorrectly EXIT_SUCCESS (ret $?)"
    fi
done

for dir in valid/* ; do
    for alg in $(seq 0 7); do
        echo "----------------------------- Testcase $dir Alg $alg -----------------------------"
        ${BINARY_TO_TEST} -a $dir/A.txt -b $dir/B.txt -o $dir/C.txt -V ${alg} 2>$dir/log.txt
        if [ $? = 0 ]; then
            # echo "+ return value is correctly EXIT_SUCCESS (ret $?). Checking stderr output..."
            if [ ! -s "$dir/log.txt" ]; then
                # echo "+ $dir/log.txt is empty! (ret $?). Checking the result matrix..."
                cmp $dir/C.txt $dir/expected.txt
                if [ $? = 1 ]; then
                    echo "BAD The result is different than expected (ret $?). Here is diff output:"
                    diff $dir/C.txt $dir/expected.txt
                # else
                #     echo "+ They are the same picture! (ret $?)"
                fi
            else
                echo "BAD $dir/log.txt is not empty! (ret $?). Here is it's content:"
                cat $dir/log.txt
            fi
        else
            echo "BAD return value is incorrectly EXIT_FAILURE (ret $?). Checking stderr output..."
            if [ ! -s "$dir/log.txt" ]; then
                echo "BAD $dir/log.txt is empty! (ret $?). CRITICAL. This should never happen!"
            else
                echo "BAD $dir/log.txt is not empty! (ret $?). Here is it's content:"
                cat $dir/log.txt
            fi
        fi
    done
done