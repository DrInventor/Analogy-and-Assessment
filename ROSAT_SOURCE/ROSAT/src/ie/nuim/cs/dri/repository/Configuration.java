/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

import java.io.IOException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * Configuration 
 * @author user
 */
public class Configuration {
   private String repositoryLocation;
   private String outputLocation;
   private String scopusAPIKey;
   private String dbName;
   private String citationDataLocation;
   private String dbUserName;
   private String dbPassWord;
    /**
     * reads the configuration file and upload the parameters
     */
    public Configuration(){
        repositoryLocation=getConfigData("ros_repository_location");
        outputLocation=getConfigData("ros_qa_output_location");
        scopusAPIKey=getConfigData("scopus_api");
        dbName=getConfigData("db_name");
        citationDataLocation=getConfigData("citation_data_location");
        dbUserName=getConfigData("db_user_name");
        dbPassWord=getConfigData("db_password");
        
    }
    /**
     * @return the repository location
    */
    public String getRepositoryLocation(){
        return repositoryLocation;
    }
     /**
     * @return the repository location
    */
    public String getDbName(){
        return dbName;
    }
    public String getDbUserName(){
        if(dbUserName.equals(""))
            dbUserName="root";
        return dbUserName;
    }
    public String getDbPassWord(){
       if(dbPassWord.equals(""))
            dbPassWord="";
        return dbPassWord;
    }
    /**
     * @return the output location    
    */
    public String getOutputLocation(){
        return outputLocation;
    }
    /**
     * @return  the Scopus API key value
     */
    public String getScopusAPIKey(){
        return scopusAPIKey;
    }
     /**
     * @return  the Scopus API key value
     */
    public String getCitationDataLocation(){
        if(citationDataLocation.equals(""))
            citationDataLocation=System.getProperty("user.dir") +"\\Citation Data";
        return citationDataLocation;
    }
    public void showData(){
       System.out.println("repository location: " +getRepositoryLocation()); 
       System.out.println("repository location: " + getOutputLocation());
       System.out.println("repository location: " + getScopusAPIKey());
       System.out.println("repository location: " + getDbName());
       System.out.println("repository location: " + getCitationDataLocation());
    }
            
     /**
     * extracts meta data from an xml file. in Dr inventor we extract the meta
     * data from xml files. this method is capable of extracting the major meta
     * data. at this stage it returns only the title of the meta data file.
     * @param xmlTag the xml tag, String
     * @return the repository location where the xml files are stored otherwise
     * returns empty
     */
    public static String getConfigData(String xmlTag) {
        String repoLocation = "";
        String xmlFile = "";
        xmlFile = System.getProperty("user.dir") + "\\config.xml";
       // System.out.println("The current file is "+ xmlFile);
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(xmlFile);
            doc.getDocumentElement().normalize();
            String rootNodeName = doc.getDocumentElement().getNodeName();
            NodeList nList;
            nList = doc.getElementsByTagName(xmlTag);
            for (int temp = 0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element eElement = (Element) nNode;
                    repoLocation = eElement.getTextContent();
                }
            }
        } catch (ParserConfigurationException | SAXException | IOException | DOMException e) {
            System.out.println("Unable to locate or read the file "+e.getMessage());
        }
        return repoLocation.trim();
    }
 
}
