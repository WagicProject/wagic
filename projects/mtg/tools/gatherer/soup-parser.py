#!/usr/bin/python
#
# Requires BeautifulSoup verion 3, available from
# http://www.crummy.com/software/BeautifulSoup/
#
# Usage soup-parser.py <xml file> [...]
#
# Updates text, subtype and name for all cards in the given xml file/s
#
# *** Overwrites the file/s given... use copies ***
#
# Copyright 2006: Nathan Callahan
# Feel free to do what you want with this file, but give credit
# where it's due.


from BeautifulSoup import BeautifulSoup, BeautifulStoneSoup, Tag
import re
import codecs
import sys
import urllib
from mtgCommon import *

stripReminderText = True

def matchNames(tag, name):
    if tag.name == 'card':
        return tag['name'].translate(nametrans) == name.translate(nametrans)
    else:
        return False

def doGathererUpdate(xml, soup):
    xmlCards = [c['name'] for c in xml.findAll('card')]
    gathererCards =[]
    
    for cardRow in soup.find(id="_gridResults").findAll('tr',onmouseover="this.style.backgroundColor='#F5DEB3';"):
        name = cardRow('b')[0].string
        name = name.replace('"','&quot;')

        htmlText = cardRow('td')[3]
        htmlText = replaceSymbols(htmlText)
        text = cleanupHTMLText(htmlText, stripReminderText)

        supertype, subtype = getCardTypes(cardRow)

        splitCard = split_re.match(text)
        if splitCard:
            text = splitCard.group('t1') + "    //    " + splitCard.group('t2')
            supertype = supertype + " // " + splitCard.group('type2')
        
        cards = xml.findAll(lambda tag: matchNames(tag, name))
        if cards:
            for card in cards:
                card['name']=name
                card['text']=text
                card.find('attr',key='type')['value']=supertype
                if subtype:
                    s = card.find('attr', key='subtype')
                    if not s:
                        card.insert(-1,Tag(xml,"attr"))
                        s = card('attr')[-1]
                        s['key'] = 'subtype'
                    s['value']=subtype
                costTag =  card.find('attr',key='cost')

                # Remove some useless attributes from previous versions
                if costTag:
                    if not costTag['value']:
                        costTag.extract()
                subtypeTag = card.find('attr',key='subtype')
                if subtypeTag:
                    if not subtypeTag['value']:
                        subtypeTag.extract()

                xmlCards.remove(name)
                    
        else:
            gathererCards.append(name)
        
    if xmlCards:
        print "Cards in file not found in gatherer:"
        print '     ' + '\n     '.join(xmlCards)
    if gathererCards:
        print "Cards in gatherer not found in file:"
        print '     ' + '\n     '.join(gathererCards)


for arg in sys.argv[1:]:
    print 'Reading:', arg
    f = file(arg)
    xml = BeautifulStoneSoup(f.read(),selfClosingTags=['attr'])
    f.close()

    setinfo = sets[xml.find('ccg-setinfo')['abbrev']]
    url = "http://ww2.wizards.com/gatherer/Index.aspx?setfilter=%s\\&output=Spoiler" % setinfo['gathname']
    print 'Fetching/Parsing:', url
    gatherer = urllib.urlopen(url)
    soup = BeautifulSoup(gatherer.read(), smartQuotesTo=None)

    print 'Processing:' + arg
    doGathererUpdate(xml,soup)

    print 'Writing:', arg
    f = file(arg,'w')
    f.write(xml.prettify('ISO-8859-1'))
    f.close()




