package powerwalk.control;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import org.powerbot.game.api.methods.Environment;
import powerwalk.Starter;
import powerwalk.model.XMLNode;

/**
 * Utility class providing methods for handling XML files
 * @author Chronio
 */
public abstract class XMLToolBox {
    
    /**
     * Returns a HashMap containing the attributes and their values from a valid XML-String
     * @param xml The XML-String to parse
     * @return a HashMap containing the attributes and values parsed from xml
     */
    public static HashMap<String,String> getAttributes(String xml) {
        HashMap<String,String> vals = new HashMap<>();
        int tagEnd = xml.indexOf(" ",xml.indexOf("<"));
        if (tagEnd == -1) {
            xml = xml.substring(xml.indexOf("<"),xml.lastIndexOf(">"));
        } else {
            xml = xml.substring(tagEnd,xml.lastIndexOf(">"));
        }
        
        while (!xml.isEmpty()) {
            try {
                xml = xml.trim();
                int is = xml.indexOf("=",0);
                if (is == -1) break;
                int openVal = xml.indexOf("'",is);
                int closeVal  = xml.indexOf("'",openVal+1);
                if (openVal == -1 || closeVal == -1) {
                    openVal = xml.indexOf("\"",is);
                    closeVal = xml.indexOf("\"",openVal+1);
                }
                String att = xml.substring(0, is);
                String val = xml.substring(openVal+1,closeVal);
                vals.put(att, val);
                xml = xml.substring(closeVal+1);
            } catch (StringIndexOutOfBoundsException e) {
                Starter.logMessage("Not the entire XML String could be parsed","ToolBox",e);
            }
        }
        return vals;
    }
    
    /**
     * Parses and returns the tag name from the given XML-String
     * @param xml the XML-String to parse the tag name for
     * @return the tag name of the XML-String
     */
    public static String getTagFromXML(String xml) {
        int index = xml.indexOf("<");
        int start = index+1;
        int end = xml.indexOf(" ",start);
        if (end == -1) end = xml.indexOf(">");
        else end = Math.min( end, xml.indexOf(">"));
        String res = xml.substring(start,end);
        if (res.startsWith("/")) res = res.substring(1,res.length());
        return res;
    }
    
    /**
     * Parses an XML Tree from the given InputStream
     * @param in the InputStream to parse an XMLTree from
     * @return the XMLTree given by the InputStream, or null when the InputStream 
     *         could not be read.
     */
    public static XMLNode getXMLTree(InputStream in) {
        ArrayList<String> lines = new ArrayList<>();
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(in))) {
            while (reader.ready()) {
                lines.add(reader.readLine());
            }
            return getXMLTree(lines,0);
        } catch (IOException iox) {
            Starter.logMessage("Could not read the InputStream","ToolBox",iox);
            return null;
        }
    }
    
    /**
     * Searches the given XMLNode for direct children that have the given attribute set to the given value.
     * @param root the root XMLNode
     * @param attribute the attribute to look for
     * @param value the value this attribute must have in order to be accepted
     * @return an array of XMLNodes matching the filter
     */
    public static XMLNode[] filterNodes(XMLNode root,String attribute,String value) {
        if (attribute == null || root == null || value == null) {
            return null;
        }
        ArrayList<XMLNode> nodes = root.children();
        ArrayList<XMLNode> matches = new ArrayList<>(nodes.size());
        for (XMLNode n : nodes) {
            String val = n.get(attribute);
            if (val != null && value.equals(val))
                matches.add(n);
        }
        return matches.toArray(new XMLNode[0]);
    }
    
    public static XMLNode[] filterNodesRecursive(XMLNode root, String attribute,String value) {
        ArrayList<XMLNode> matches = new ArrayList<>(Arrays.asList(filterNodes(root, attribute, value)));
        for (XMLNode n : root.children()) {
            matches.addAll(Arrays.asList(filterNodesRecursive(n, attribute, value)));
        }
        return matches.toArray(new XMLNode[0]);
    }
    /**
     * Creates an XML Tree from the given list of lines, where every String element represents one XML Tag
     * @param lines a List of XML Strings
     * @return an XML tree created from the list of XML Strings
     */
    public static XMLNode getXMLTree(ArrayList<String> lines) {
        return getXMLTree(lines,0);
    }
    
    private static int lineIndex = 0;
    private static XMLNode getXMLTree(ArrayList<String> lines, int start) {
        try {
            lineIndex = start;
        
        String root = lines.get(start);
        if (root.contains("<!--") ) { // ignore comments
            while (!lines.get(lineIndex).contains("-->")) {
                lineIndex++;
                if (lineIndex >= lines.size()) break;
            }
            return null;
        }
        if (root.isEmpty() ||                                 // ignore empty lines
           (root.contains("<?") && root.contains("?>")) ||    // ignore <?xml ... ?> tag
           (root.contains("<!")))                             // ignore <! ... > tags (DTD and comments
            return getXMLTree(lines,start+1);
        
        HashMap<String,String> attributes = getAttributes(root);
        String tag = getTagFromXML(root);
        ArrayList<XMLNode> children = new ArrayList<>(5);
        if (!root.contains("/>")) {
            lineIndex++;
            for (;lineIndex<lines.size();lineIndex++) {
                String current = lines.get(lineIndex);
                if (current.contains("</" + tag + ">")) {
                    break;
                }
                XMLNode n = getXMLTree(lines,lineIndex);
                if (n != null) {
                    children.add(n);
                }
            }
        } else {
            children = null;
        }
        return new XMLNode(tag,attributes,children);
        } catch (Exception e) {
            Starter.logMessage("Error while parsing XML tree","ToolBox",e);
            return null;
        }
    }
    
    public static boolean writeToFile(Object o, String file) {
        String text = String.valueOf(o);
        try {
            String fullpath = Environment.getStorageDirectory().toString() + "\\" + file;
            
            File target = new File(fullpath);
            if (!target.exists()) {
                target.createNewFile();
                Starter.logMessage("New File created at " + target.getAbsolutePath(),"ToolBox");
            }
            try (FileWriter w = new FileWriter(target)) {
                w.write(text);
            }
            return true;
        } catch (IOException e) {
            Starter.logMessage("Error writing to file", "ToolBox", e);
            return false;
        }
    }
}
