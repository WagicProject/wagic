#! /usr/bin/env python

"""Basic tests to ensure pyjavaproperties behaves like java.util.Properties.

Created - Pepper Lebeck-Jobe (eljobe@gmail.com)
"""

import os
import unittest

from pyjavaproperties import Properties


class PyJavaPropertiesTest(unittest.TestCase):
  """Tests pyjavaproperties complies to java.util.Properties contract."""

  def setUp(self):
    test_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testdata')
    self.properties_file = os.path.join(test_dir, 'complex.properties')

  def testParsePropertiesInput(self):
    properties = Properties()
    properties.load(open(self.properties_file))
    self.assertEquals(23, len(properties.items()))
    self.assertEquals('Value00', properties['Key00'])
    self.assertEquals('Value01', properties['Key01'])
    self.assertEquals('Value02', properties['Key02'])
    self.assertEquals('Value03', properties['Key03'])
    self.assertEquals('Value04', properties['Key04'])
    self.assertEquals('Value05a, Value05b, Value05c', properties['Key05'])
    self.assertEquals('Value06a, Value06b, Value06c', properties['Key06'])
    self.assertEquals('Value07b', properties['Key07'])
    self.assertEquals(
        'Value08a, Value08b, Value08c, Value08d, Value08e, Value08f',
        properties['Key08'])
    self.assertEquals(
        'Value09a, Value09b, Value09c, Value09d, Value09e, Value09f',
        properties['Key09'])
    self.assertEquals('Value10', properties['Key10'])
    self.assertEquals('', properties['Key11'])
    self.assertEquals('Value12a, Value12b, Value12c', properties['Key12'])
    self.assertEquals('Value13 With Spaces', properties['Key13'])
    self.assertEquals('Value14 With Spaces', properties['Key14'])
    self.assertEquals('Value15 With Spaces', properties['Key15'])
    self.assertEquals('Value16', properties['Key16 With Spaces'])
    self.assertEquals('Value17', properties['Key17 With Spaces'])
    self.assertEquals('Value18 # Not a comment.', properties['Key18'])
    self.assertEquals('Value19 ! Not a comment.', properties['Key19'])
    self.assertEquals('Value20', properties['Key20=WithEquals'])
    self.assertEquals('Value21', properties['Key21:WithColon'])
    self.assertEquals('Value22', properties['Key22'])

if __name__ == '__main__':
  unittest.main()
