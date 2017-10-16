----------------------
File: readme.txt
Assignment: assign0 Scavenger Hunt
Author: Tiantian Tang
----------------------

----------------------
QUESTION: Describe three of the most useful things you've learned 
to do in UNIX. Write one sentence for each. Write as though you're 
trying to sell a classmate on the utility of learning this particular 
command/trick/tip.
ANSWER:
1. ls : to see what is in the current folder. simple but powerful. what if your current folder store the route/map of a hidden treasure? :)
2. history: to see what happened in the past! so if you make mistakes, you can go back and learn from it!
3. cat filename: to view a file quickly! Anyone need this to read the guid to hidden treasure!


----------------------
QUESTION: Describe three of the most useful things you've learned 
to do in your editor of choice (either vim or emacs--you don't need
to write about both vim and emacs). Write one sentence for each useful
thing you learned. Write as though you're trying to sell a classmate 
on the utility of learning this particular command/trick/tip.
ANSWER:
I used vim
1. vim -o file1 file2: use this to edit 2 files at same time with two horizontal windows. Like to multi-task? You need this!
2. ctrl + h:  delete the previous word where cursors is located. You don't want to waste time on keep pressing delete button to remove word like this, pneumonoultramicroscopicsilicovolcanoconiosis, right?
3. :%s/oldpattern/newpattern/gc: to replace all the old patterns with new patterns. Pretty useful when you want to rename a function name in code.


----------------------
QUESTION: According to CS107's Collaboration and Honor Code Policy on 
the course website (see link in the assign0 webpage), is helping 
another student debug their code by listening to them verbally describe 
the symptoms and then making suggestions: (A) Assistance that is` allowed 
and requires no citation, (B) Assistance that is allowed but must be 
cited, or (C) Assistance that is NOT allowed? (write letter choice below)
ANSWER: 
B

----------------------
QUESTION: How many times did the number "111" occur?
ANSWER: 5

----------------------
QUESTION: What is the username of the intruder?
ANSWER:mvaska 

----------------------
QUESTION: What is the date/time when the trusted.list file was changed?
ANSWER: Jan 10 18:52, 2017 

----------------------
QUESTION: List the names of the programs in the server_image_91107/bin/ 
directory that appear (based on timestamp) to have been edited by the
intruder. 
ANSWER:baash, rkit, shellh 

----------------------
QUESTION: List the usernames of the all the users whose init.d files 
appear to have been compromised by the intruder.
ANSWER: fennelturnips, observantben,woolerduty,unfastencutter,yangoncurrency,poundreflect
----------------------




































































































































































































































































































































































































































































































In the directory unixhunt/nums, there are two files full of numbers. Take a moment to look at the first and last few lines (using the Unix 'head' and 'tail' commands, respectively) to see for yourself (e.g., "head -20 nums1.txt").

The two files are identical except for the presence of one additional line in one that is not in the other. Find this line that is in one file and not the other and remember what number is written on that line (not the line number of the file, but the what is written there, which is a number). We'll call that number X. Now open the m_map.txt file that is in your cloned assign0 directory (alongside this readme.txt file), and write "I solemnly swear that I am up to no good" on line number X of that file. You should use your editor "protip" tricks to jump directly to line number X!

*Note*: When you submit your m_map.txt for grading, this text should be on line number X exactly, so double-check before submitting that you left it exactly there. 

Count how many times the number 111 occurs in the nums1.txt file. Write that number in response to the appropriate question above in this readme.txt file.

NOW RETURN TO THE ASSIGN0 WEB PAGE FOR INSTRUCTIONS ON THE NEXT ACTIVITY (intruder detection)!

