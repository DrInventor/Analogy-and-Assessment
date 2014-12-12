/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.metadata;

import ie.nuim.cs.dri.repository.Configuration;
import java.awt.Frame;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Arrays;
import java.util.Calendar;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import ie.nuim.cs.dri.repository.ROS;
import ie.nuim.cs.dri.repository.ROSMetadata;
import ie.nuim.cs.dri.repository.TitleComparator;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * This class is used to instantiate classes and methods of other classes It
 * serves as a gateway to the whole drInventor metadata extraction and ROS
 * quality assessment
 *
 * @author Yalemisew
 */
public class MetadataExtractor extends Frame {

    /**
     *
     */
    public static String ROSforDB;

    /**
     * is the executable method of the class and instanciates methods of other
     * classes reads files from a directory, in this case the directory is a
     * fixed location user can supply the directory using the main method
     * arguments two directories can be used interchangeably one with 120 papers
     * and the other with 6 papers for test purposes
     *
     * @param args
     * @throws InterruptedException
     * @throws Exception
     */
    public static void main(String[] args) throws InterruptedException, Exception {
        DirectoryReader dirReader = new DirectoryReader();
        Configuration config= new Configuration();
        String repositoryLocation = config.getRepositoryLocation();
        String outputLocation = config.getOutputLocation();
        dirReader.readXMLFiles(repositoryLocation);
        int fileCount = dirReader.getXmlFileList().size();

        System.out.println("Processing " + fileCount + " Files");
        String xmlTitle = "";
        String xmlString = "";
        WebSearch websearch = new WebSearch();
        ROS ros = new ROS();
        ROS[] allROS;
        ROSMetadata sm = new ROSMetadata();
        /**
         * reads an xml file and extract the title of the ROS from the xml data.
         */
        allROS = new ROS[dirReader.getXmlFileList().size()];
        int i = 0;
        for (String file : dirReader.getXmlFileList()) {
            xmlString = "";
            xmlTitle = extractTitle(file);
            
            //metadata searching
            //searchs the metadat form scopus and gets an xml file
            System.out.println("\nSearching scopus..." + xmlTitle);
            Thread.sleep(100);// test.searchGoogle(xmlTitle);
            //call to scopus search and extractthe returning xml data using the extractor and assigns to the ros
            ros = WebSearch.extractScopusMetadata(websearch.searchScopus(xmlTitle), xmlTitle);
            //if the article is not found from the providers, we keep its title form the xml file and assign the value.
            //it is used to track unsuccessful metadata search
           
            if (ros.getArticleTitle() == "") {
                System.out.println("\nSwitch to CiteSeerX..." + xmlTitle);
                ros = WebSearch.extractCiteSeerMetadata(websearch.searchCiteSeer(xmlTitle), xmlTitle);
            }
            if (ros.getArticleTitle() == "") {
                ros.setArticleTitle(xmlTitle);
            }
            //Metrics assignment
            //for each ROS the following code assignes the metrics. Some of these metrics are stored in a database (Impact Factors, Hindex, institution rank conference ranks)
            ros.setJournalIF(sm.getImpactFactor(ros));
            ros.setJournalHI(sm.getHindex(ros));
            ros.setInstRanking1(sm.getInstitutionRank(ros));
            //recheck whether the identified ROS is a journal or not
            ros.recheckPublicationType();
            int[] confRanks = new int[2];
            confRanks = sm.getConferenceRank(ros);
            ros.setConferenceRankingFR(confRanks[0]);
            ros.setConferenceRankingC(confRanks[1]);
            ros.showFullData();
            allROS[i] = new ROS();
            allROS[i] = ros;
            i++;
            //saving
            // preparing the metadata to be saved as a CSV file and send each ROS to a database
            sm.writeToScopusDatabase(ros);
            // prepareForCSVFile(ros);
            // prepareForARFFFile(ros);

        }
        
        Arrays.sort(allROS, new TitleComparator());

        if (allROS != null) {
            Long now = Calendar.getInstance().getTimeInMillis();
            writeToFile(outputLocation + "\\ROSinCSV_" + fileCount + "_" + now.toString() + ".csv", allROS[0].prepareForCSVFile(allROS) );
            writeToFile(outputLocation + "\\ROSinARFF_" + fileCount + "_" + now.toString() + ".arff",allROS[0].prepareForARFFFile(allROS) );
        }

    }

    /**
     * writes the ros to a csv file
     *
     * @param fileName the file name and location the comma separated ros will
     * be stored
     * @param content
     */
    public static void writeToFile(String fileName, String content) {
        Writer writer = null;
        try {
            writer = new BufferedWriter(new OutputStreamWriter(
                    new FileOutputStream(fileName)));
            writer.write(content);
            System.out.println("the file is successfully written");
        } catch (IOException ex) {
            System.out.println(ex.getMessage());
        } finally {
            try {
                writer.close();
            } catch (Exception ex) {
            }
        }
    }

   
    /**
     * extracts the title of the XML file passed to it by the parameter
     *
     * @param xmlFile the location and the name of the ROS
     * @return the title of the ROS
     */
    public static String extractTitle(String xmlFile) {
        String title = "";
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(xmlFile);
            doc.getDocumentElement().normalize();
            NodeList nList;
            nList = doc.getElementsByTagName("article-title");
            for (int temp = 0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element eElement = (Element) nNode;
                    title = eElement.getTextContent();
                }
            }

        } catch (ParserConfigurationException | SAXException | IOException | DOMException e) {
            System.out.println(e.getMessage());
        }
        return title;
    }

}
