# Learncards #

Description
===========
Learncards is a tiny anki-like flashcard application for use in your terminal. Written in - almost ANSI - C

Helps you to study your favourite subjects in a very efficient way.

It is simple but more fun.


You can create your own packs/databases and edit or add new cards easily.

If you are in rush you can also add just one front side of a card,

and during your training you can also edit the back side.

The packs are simple text files you can also edit them with your favourite editor.

Put your packs into your favourite version control system.

Share your favourite packs with your friends send them via email or slack

or make it even public eg. on github or bitbucket.

Open source
===========
https://github.com/szilveszter9/learncards

Install and run the tutorial
============================
* If you are happy with the provided binary
```shell
git clone https://github.com/szilveszter9/learncards
cd learncards
./anki
```

My pack example
===============
* There is an example 'en-hu' database - I've built for my own purposes.
I start my training with the following command:
```shell
./anki en-hu
```
When I want to stop the training I just close the application by pressing 'q' at any point.

Create your own pack
====================
```shell
./anki i my_new_cards  -  create a new empty database if it doesn't exists yet
./anki a my_new_cards  -  add new cards to your database
```

Build your own pack with your favourite editor
==============================================
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

Build it your own
=================
```shell
git clone https://github.com/szilveszter9/learncards
cd learncards
gcc anki.c -Wall -o anki
./anki
```

Contribution
============
Bug fixes, docs and examples are welcomed.
