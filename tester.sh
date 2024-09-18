# cat << EOF
# run
# ls
# EOF | ./expertsystem example-input1.txt 

echo -e "fact\nDEIJOP\nrun\nfact\nDEIJP\nrun" | ./expertsystem example-input1.txt 

echo ========================test2====================
sleep 1
echo -e "fact\n\nrun\nfact\nD\nrun\nfact\nE\nrun\nfact\nDE\nrun" | ./expertsystem example-input2.txt 

echo ========================test3====================
sleep 1
echo -e "fact\n\nrun\nfact\nD\nrun\nfact\nE\nrun\nfact\nDE\nrun" | ./expertsystem example-input3.txt 

echo ========================test4====================
sleep 1
echo -e "fact\n\nrun\nfact\nB\nrun\nfact\nC\nrun\nfact\nBC\nrun" | ./expertsystem example-input4.txt 

echo ========================test5====================
sleep 1
echo -e "fact\n\nrun\nfact\nB\nrun\nfact\nC\nrun\nfact\nBC\nrun" | ./expertsystem example-input5.txt 

echo ========================test6====================
sleep 1
echo -e "fact\n\nrun\nfact\nA\nrun\nfact\nB\nrun\nfact\nC\nrun\nfact\nAC\nrun\nfact\nBC\nrun\nfact\nF\nrun\nfact\nG\nrun\nfact\nH\nrun\nfact\nFH\nrun\nfact\nGH\nrun" | ./expertsystem example-input6.txt 