#! /bin/bash

for ((i=0; i<20; i++))
do
    GET -dUe http://ec2-54-218-67-46.us-west-2.compute.amazonaws.com:3005/test4.jpg &
done
