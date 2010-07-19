Documentation for the Basic campaign system in Wagic

Each subfolder that contains a "story.xml" is considered as a "story". Stories appear in the menu and represent
a series of dialogs and duels.

*: 0 to n
?: 0 to 1

#################################
 1.Structure of story.xml:
#################################

===========================================
Elements of the root node
===========================================

page * 
  attributes:
     id (compulsory)

===========================================
Elements of the page node
===========================================

type (dialog,duel,end)
music ? (path to a music file)


===========================================
Additional Elmts of the "page" node if type == dialog
===========================================

title *
img *
text *
answer *
reward *

title, img, text, answer have the following common attributes:
x (optional - default 0)
  0 means auto
  0 < x < 1 means percentage of screen width
  other value = absolute position
y (optional - default 0)
  -1 means: go to previous line
  0 means: auto
  0 < y < 1 means: percentage of screen height
  other value = absolute position
font (optional - default 0)
    0, 1, or 2
align (optional - default left)
   left, center, right


font and align not supported by title, which is equivalent to <text font="1" align="center">

answer specific attributes:
goto
  id of a page to go to

reward attributes:
type
    unlockset , card, credits
value
   number of credits, or name/id of the card (random if undef), or name of the set (random if undef)
Additionally, the fowllowing variables can be used in the reward text : ${SET}, ${CARD}

===========================================
Additional Elmts of the "page" node if type == duel
===========================================
onwin
  id of a page to go to if duel lost
onlose
  id of a page to go to if duel won
bg
 path to a background file



#################################
 2.Duel mode
#################################