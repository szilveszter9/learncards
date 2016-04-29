# Learncards #

Description
===========
Learncards is a tiny anki-like flashcard application for use in your terminal. Written in - almost ANSI - C

Github
======
https://github.com/szilveszter9/learncards

Install and run the tutorial
============================
* If you'd like to build it your own:
```shell
git clone https://github.com/szilveszter9/learncards
cd learncards
gcc anki.c -Wall -o anki
./anki
```
* If you are happy with the provided binary
```shell
git clone https://github.com/szilveszter9/learncards
cd learncards
./anki
```

Start with your own cards
=========================
* There is an en-hu database I've built for my own purposes.
I run it like this:
```shell
./anki en-hu
```

Build your own pack
===================
```text
You can create a database simply by creating a text file.

Eg. your database name is:   study
then you have to create
a text file with the name:  study_db.txt

The content should be like:
(without any empty lines at the beginning or at the end)

;;question 1;answer 1
;;question 2;answer 2
;;question 3;answer 3

After all you just run:
./anki study
```

Accents
=======
The application is capable to handle accents
though the actual representation might be different
based on your terminal capabilities.
Try:
```shell
./anki accents
```
A great place to find accents by languages:
http://www.typeit.org/

Contribution
============
Bug fixes, docs and examples are welcomed.
