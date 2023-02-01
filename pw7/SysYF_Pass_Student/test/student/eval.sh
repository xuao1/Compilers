echo "enter Optimization, current dir is:"
pwd
# write your evaluation script here, please use relative path instead of absolute path!!!

## example: compile your project
## 1. build project
cd ../../
mkdir -p build
cd build
cmake ../
cmake --build .

echo "build done"

## 2. execute and collect result

## 测试助教的样例

echo ""
echo "begin to run the official test"
echo ""


cd ../test
python3 test.py

echo ""

cd ../build

list=`ls ../test/student/sy`

outputPath=../test/student/output/

mkdir -p $outputPath

echo "begin to compile student's source file and collect outputs & times."

nameList=()
testList=()
timeOptList=()
timeNoOptList=()
declare -i n=0
declare -i j=0

for file in $list
do
    optOptList=`echo ${file:0:${#file[0]}-3} | tr "_" "\n"`
    optOpt="-O"
    for o in $optOptList
    do
        if test $o != "O2"
        then
            optOpt="$optOpt -$o"
        else
            optOpt="-O2"
            break
        fi
    done
    syFile="../test/student/sy/$file"
    # echo "./compiler -emit-ir $syFile -o noopt.ll"
    ./compiler -emit-ir $syFile -o noopt.ll
    # echo "./compiler -emit-ir $syFile -o opt.ll $optOpt"
    ./compiler -emit-ir $syFile -o opt.ll $optOpt
    clang noopt.ll -o noopt -w
    clang opt.ll -o opt -w

    ./compiler -emit-ir $syFile -o "${outputPath}${file:0:${#file[0]}-3}_noopt.ll" -O
    cp opt.ll "${outputPath}${file:0:${#file[0]}-3}_opt.ll"

    start1=$(date +%s%N)
    for((j=0;j<10;j++))
    do
        # echo "$syFile noopt $j"
        ./noopt
    done
    end1=$(date +%s%N)
    take1=$(( (end1 - start1)/10 ))

    start2=$(date +%s%N)
    for((j=0;j<10;j++))
    do
        # echo "$syFile opt $j"
        ./opt
    done
    end2=$(date +%s%N)
    take2=$(( (end2 - start2)/10 ))

    output1=`./noopt;echo $?`
    output2=`./opt;echo $?`

    nameList[n]=$file
    if test $[output1] -eq $[output2]
    then
        testList[n]="true"
    else
        testList[n]="false"
    fi
    timeNoOptList[n]=$take1
    timeOptList[n]=$take2
    ((++n))
done

echo ""

output="fileName outputEq optTime(ns) noOptTime(ns)\n"
for((i=0;i<n;i++));
do
output="$output ${nameList[i]} ${testList[i]} ${timeOptList[i]} ${timeNoOptList[i]}\n";
done

echo -e $output | column -t

echo -e '\nyou can check the output/ to diff the outputs between the "-O" and the optimize option figured in the name of file\n'