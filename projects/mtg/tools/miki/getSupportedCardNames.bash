#!/bin/bash

####################################################
#  adjust the values for mtg.txt and borderline.txt
#  if your "Res" directory isn't in the default place
####################################################

####################################################
#  Purpose:  build an HTML list of all the supported 
#	cards in Wagic for use in the Miki.
#	http://wololo.net/.
####################################################
grep '^name=' ../../bin/Res/sets/primitives/mtg.txt ../../bin/Res/sets/primitives/borderline.txt|sed s/name=// | sed s/.*txt:// |sort -iu > supportedCards.txt

perl createHTMLList_SupportedCards.pl < supportedCards.txt 
