import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

String android_manifest_filename=args[0];
File fXmlFile = new File(android_manifest_filename);
DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
Document doc = dBuilder.parse(fXmlFile);

Element rootNode=doc.getDocumentElement();

/*
      package="net.wagic.app"   
      android:versionCode="184"
      android:versionName="0.18.4"
*/


def props = new Properties();
new File("build.number.properties").withInputStream { 
  stream -> props.load(stream) 
}
// accessing the property from Properties object using Groovy's map notation
println "capacity.created=" + props["build.major"]

def config = new ConfigSlurper().parse(props)
// accessing the property from ConfigSlurper object using GPath expression


String major_version=props["build.major"];
String minor_version=props["build.minor"];
String point_version=props["build.point"];

DOMSource source = new DOMSource(doc);

rootNode.setAttribute("android:versionName", major_version + "." + minor_version + "." + point_version);
rootNode.setAttribute("android:versionCode", major_version + minor_version + point_version);
File outputFile = new File(android_manifest_filename );
TransformerFactory tFactory = TransformerFactory.newInstance();
Transformer transformer = tFactory.newTransformer();

transformer.setOutputProperty("indent", "yes");

StreamResult result = new StreamResult(new FileOutputStream(outputFile) );
transformer.transform(source, result);
