#! /bin/bash

for ((i=0; i<20; i++))
do
    GET -dUe http://ec2-54-148-248-229.us-west-2.compute.amazonaws.com:3000/test4.jpg &
done
