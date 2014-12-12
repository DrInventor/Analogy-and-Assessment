/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.metadata;

import ie.nuim.cs.dri.repository.Configuration;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import ie.nuim.cs.dri.repository.ROS;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 *
 * @author Yalemisew
 */
public class WebSearch {

    /**
     * Searches the Scopus database
     *
     * @param title the title of the research object
     * @return returns an xml response of Scopus
     */
    public String searchScopus(String title) {
        String xmlString = "";
        Configuration config = new Configuration();
        String scopusAPI = config.getScopusAPIKey();
       
        try {
            // create default HTTP Client
            CloseableHttpClient httpClient = HttpClientBuilder.create().build();

            // Create new getRequest with below mentioned URL
            String query = title;
            //uncomment the following line if you want to make an exact search, very restrictive
            //query = buildScopusSExactearchTitle(query);
            query = buildScopusSearchTitle(query);
            HttpGet getRequest = new HttpGet("http://api.elsevier.com/content/search/index:scopus?query=" + query + "&count=50");
            //System.out.println("http://api.elsevier.com/content/search/index:scopus?query=" + query + "&count=50");

            // Add additional header to getRequest which accepts application/xml data
            getRequest.addHeader("X-ELS-APIKey", scopusAPI);
            getRequest.addHeader("X-ELS-ResourceVersion", "XOCS");
            getRequest.addHeader("accept", "application/xml");

            // Execute your request and catch response
            HttpResponse response = httpClient.execute(getRequest);

            // Check for HTTP response code: 200 = success
            if (response.getStatusLine().getStatusCode() != 200) {
                throw new RuntimeException("Failed : HTTP error code : " + response.getStatusLine().getStatusCode());
            }

            // Get-Capture Complete application/xml body response
            BufferedReader br = new BufferedReader(new InputStreamReader((response.getEntity().getContent())));
            String output;
            while ((output = br.readLine()) != null) {
                xmlString += output;

            }
            httpClient.close();

        } catch (ClientProtocolException e) {
        } catch (IOException e) {
        }
        //System.out.println("Scopus Search Result=\n" + xmlString);
        return xmlString;
    }

   
    /**
     *
     * @param title the title of the ROS
     * @return the search result of WOS, xml formatted data
     */
    public String searchWOS(String title) {
        String xmlString = "";
        try {
            // create default HTTP Client
            CloseableHttpClient httpClient = HttpClientBuilder.create().build();

            // Create new getRequest with below mentioned URL
            String query = title;
            query = buildScopusSearchTitle(query);
            HttpGet getAuthRequest = new HttpGet("http://search.webofknowledge.com/esti/wokmws/ws/WOKMWSAuthenticate");
            HttpResponse response = httpClient.execute(getAuthRequest);

            HttpGet getSearchRequest = new HttpGet("http://search.webofknowledge.com/esti/wokmws/ws/WOKMWSAuthenticate");

            // Add additional header to getRequest which accepts application/xml data
            getSearchRequest.addHeader("SID", response.toString());

            // Execute your request and catch response
            HttpResponse searchResponse = httpClient.execute(getSearchRequest);

            // Check for HTTP response code: 200 = success
            if (response.getStatusLine().getStatusCode() != 200) {
                throw new RuntimeException("Failed : HTTP error code : " + response.getStatusLine().getStatusCode());
            }

            // Get-Capture Complete application/xml body response
            BufferedReader br = new BufferedReader(new InputStreamReader((response.getEntity().getContent())));

            //System.out.println("============Output:============");
            String output;
            // Simply iterate through XML response and show on console.
            while ((output = br.readLine()) != null) {
                xmlString += output;

            }

            //httpClient.getConnectionManager().shutdown();
            httpClient.close();

        } catch (ClientProtocolException e) {
        } catch (IOException e) {
        }
        //System.out.println("WOS Search Result=" + xmlString);
        return xmlString;
    }

    /**
     *
     * @param title the title of the ROS
     */
    public void searchGoogle(String title) {
        String searchTitle = buildGoogleSearchTitle(title);
        boolean found = false;
        String publication = "";
        String publicationType = "";
        int citationCount = -1;
        String url = "http://scholar.google.com/scholar?" + searchTitle;
        Document doc = Jsoup.parse(getGS());
        Elements aElement = doc.getElementsByTag("h3");
        System.out.println("=====searching google=======");
        for (Element e : aElement) {
            Elements bElement = e.getElementsByTag("a");
            for (Element f : bElement) {
                System.out.println(f.text() + "\t" + title);

                if (title.equalsIgnoreCase(f.text())) {
                    found = true;
                    break;
                }
            }
            // System.out.println(e);

        }
        if (found == true) {
            Elements pElement = doc.getElementsByTag("div");
            for (Element p : pElement) {
                Elements pubElement = p.getElementsByClass("gs_a");
                for (Element pub : pubElement) {
                    System.out.println(pub);
                }

            }
            for (Element p : pElement) {
                Elements pubElement = p.getElementsByClass("gs_fl");
                for (Element pub : pubElement) {
                    System.out.println(pub);
                }

            }

        }
    }

    /**
     *
     * @param title
     * @return
     */
    public String searchCiteSeer(String title) {
        String htmlString = "";
        try {
            String url = "http://citeseerx.ist.psu.edu/search?" + buildCiteSeerSearchTitle(title);
            Document doc = Jsoup.connect(url)
                    .timeout(30000)
                    .get();
            htmlString = doc.toString();
            // Document doc = Jsoup.parse(getGS());
        } catch (IOException ex) {
           // System.out.println("The server took longer than usual to respond, please try again later on. "+ ex.getLocalizedMessage() +"\n"+ex.getCause());
           Logger.getLogger(WebSearch.class.getName()).log(Level.SEVERE, null, ex);
        }
        return htmlString;
    }

    /**
     *
     * @param title
     * @return
     */
    public static String buildCiteSeerSearchTitle(String title) {
        //q = "title%3A%28{1}%29+author%3%28{0}%29&submit=Search&sort=cite&t=doc"
        String titleTokens[] = title.split(" ");
        String searchTitle = "q=title%3A%28";
        for (int i = 0; i < titleTokens.length - 1; i++) {
            searchTitle += titleTokens[i] + "+";
        }
        searchTitle += titleTokens[titleTokens.length - 1] + "%29&submit=Search&sort=cite&t=doc";
        return searchTitle;

    }

    /**
     *
     * @param title
     * @return
     */
    public static String buildGoogleSearchTitle(String title) {
        String titleTokens[] = title.split(" ");
        String searchTitle = "q=";
        for (int i = 0; i < titleTokens.length - 1; i++) {
            searchTitle += titleTokens[i] + "+";
        }
        searchTitle += titleTokens[titleTokens.length - 1] + "%22%29";
        // System.out.println("The search title is :" + searchTitle);
        return searchTitle;

    }

    /**
     *
     * @param title
     * @return
     */
    public static String buildScopusSearchTitle(String title) {
        String titleTokens[] = title.split(" ");
        String searchTitle = "TITLE%28%22";
        for (int i = 0; i < titleTokens.length - 1; i++) {
            searchTitle += titleTokens[i] + "%20";
        }
        searchTitle += titleTokens[titleTokens.length - 1] + "%22%29";
        // System.out.println("The search title is :" + searchTitle);
        return searchTitle;

    }

    /**
     *
     * @param title
     * @return
     */
    public static String buildScopusExactSearchTitle(String title) {

        String titleTokens[] = title.split(" ");
        String searchTitle = "TITLE%7B";
        for (int i = 0; i < titleTokens.length - 1; i++) {
            searchTitle += titleTokens[i].trim() + "%20";
        }
        searchTitle += titleTokens[titleTokens.length - 1] + "%7D";
        System.out.println("The search title is :" + searchTitle);
        return searchTitle;

    }

    /**
     *
     * @param title
     * @return
     */
    public static String buildNatureSearchTitle(String title) {
        String titleTokens[] = title.split(" ");
        String searchTitle = "";
        for (int i = 0; i < titleTokens.length - 1; i++) {
            searchTitle += titleTokens[i] + "+";
        }
        // searchTitle += titleTokens[titleTokens.length - 1] + "%22%29";
        // System.out.println("The search title is :" + searchTitle);
        return searchTitle;

    }

    /**
     *
     * @return
     */
    public String getGS() {
        BufferedReader br = null;
        String sCurrentLine;
        String fullText = "";
        try {
            FileReader jrcFileReader = new FileReader("C:\\DRINVENTOR_PROJECT\\corpus\\test.htm");
            br = new BufferedReader(jrcFileReader);
            StringBuilder sb = new StringBuilder();

            while ((sCurrentLine = br.readLine()) != null) {
                sb.append(sCurrentLine);
                //System.out.println(sb.toString());
                sb.append(System.lineSeparator());

            }
            fullText = sb.toString();
        } catch (IOException e) {
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
            } catch (IOException ex) {
            }
        }
        //System.out.println("Full text from gs "+ fullText);
        return fullText;
    }

    /**
     * extracts the field from a given xml string
     *
     * @param xmlTag the name of the tag used in the xml file (such as title,
     * author etc)
     * @param xmlString the xml string
     * @return the extracted value of the tag
     */
    public static String extractField(String xmlTag, String xmlString) {
        String extractedField = "";
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            org.w3c.dom.Document doc = dBuilder.parse(new InputSource(new StringReader(xmlString)));
            doc.getDocumentElement().normalize();
            String rootNodeName = doc.getDocumentElement().getNodeName();
            NodeList nList;
            nList = doc.getElementsByTagName(xmlTag);
            for (int temp = 0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                //System.out.print("\nCurrent Element:" + nNode.getNodeName());
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    org.w3c.dom.Element eElement = (org.w3c.dom.Element) nNode;
                    extractedField = eElement.getTextContent();
                }
            }
        } catch (ParserConfigurationException | SAXException | IOException ex) {
            Logger.getLogger(MetadataExtractor.class.getName()).log(Level.SEVERE, null, ex);
        }
        extractedField = extractedField.replace(",", ";");
        return extractedField;
    }

    /**
     * extracts multiple fields from the xml string and assign the values to a
     * ROS instance. It iterates through all instances and
     *
     * @param xmlString the xml String
     * @return an array of ROS
     */
    public static ROS[] extractMultiField(String xmlString) {
        ROS[] ros = null;
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            org.w3c.dom.Document doc = dBuilder.parse(new InputSource(new StringReader(xmlString)));
            doc.getDocumentElement().normalize();
            String rootNodeName = doc.getDocumentElement().getNodeName();
            NodeList nList;
            nList = doc.getElementsByTagName("entry");
            ros = new ROS[nList.getLength()];
            for (int temp = 0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                ros[temp] = new ROS();
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    org.w3c.dom.Element eElement = (org.w3c.dom.Element) nNode;
                    NodeList childNodes = nNode.getChildNodes();
                    for (int i = 0; i < childNodes.getLength(); i++) {
                        Node childNode = childNodes.item(i);
                        if (childNode.getNodeType() == Node.ELEMENT_NODE) {
                            org.w3c.dom.Element childElement = (org.w3c.dom.Element) childNode;
                            //System.out.println(childNode.getNodeName() +"\t"+ childElement.getTextContent());
                            String nodeValue = childElement.getNodeName();
                            String value = childElement.getTextContent();

                            if (nodeValue == "prism:doi") {
                                ros[temp].setDoi(value.trim());
                            } else if (nodeValue == "dc:title") {
                                ros[temp].setArticleTitle(value.trim());
                            } else if (nodeValue == "prism:publicationName") {
                                ros[temp].setPublicationName(value.trim());
                            } else if (nodeValue == "prism:issn") {
                                ros[temp].setIssn(value.trim());
                            } else if (nodeValue == "affiliation") {
                                ros[temp].setAffiliation1(childElement.getElementsByTagName("affilname").item(0).getTextContent());
                            } else if (nodeValue == "prism:coverDisplayDate") {
                                ros[temp].setYear(extractYearFromYearString(value));
                            } else if (nodeValue == "citedby-count") {
                                ros[temp].setCitedByCount(Integer.parseInt(value.trim()));
                            } else if (nodeValue == "prism:aggregationType") {
                                ros[temp].setPublicationType(value.trim());

                            } else if (nodeValue == "prism:volume") {
                                ros[temp].setVolume(value.trim());
                            }

                        }

                    }

                }
                // ros[temp].showMetaData();
            }
        } catch (ParserConfigurationException | SAXException | IOException ex) {
            Logger.getLogger(MetadataExtractor.class.getName()).log(Level.SEVERE, null, ex);
        }

        return ros;
    }

    /**
     * this method extracts the publication which is a journal and sum up the
     * citation from the other sources We choose a journal paper if there exist
     * one, otherwise the first ros. We cross checked the citation and summing
     * up all the citations is a logical approach as it doesn't go above the
     * Google scholar's citation count (for example v1 20 cites v2 17 cites and
     * the paper earn 37 cites.)
     *
     * @param allRos an array representing the different version of the same ROS
     * appearing in conference, journal book series etc
     * @param title the title of the research object skeleton
     * @return
     */
    public static ROS extractBestROS(ROS[] allRos, String title) {
        boolean isJournal = false;
        String properPubName = "";
        int targetRos = -1;
        int jorTargetRos = -1;
        int totalCitation = 0;
        for (int rosCount = 0; rosCount < allRos.length; rosCount++) {
            //checks if the article title matches and if the entry is a journal
            if (allRos[rosCount].getArticleTitle().equalsIgnoreCase(title)) {
                if (allRos[rosCount].getPublicationType().equalsIgnoreCase("journal")) {
                    isJournal = true;
                    jorTargetRos = rosCount;
                } else {
                    targetRos = rosCount;
                }
                // System.out.println("found the title");
                targetRos = rosCount;

                totalCitation += allRos[rosCount].getCitedByCount();

                //checks if the publicaiton  name is an official name, if it contains issn number, it is potentially a proper publication name
                if ((!allRos[rosCount].getIssn().equalsIgnoreCase("")) && (!allRos[rosCount].getVolume().equalsIgnoreCase(""))) {
                    //System.out.println("**************\n\nbest is found at "+rosCount+"\n\n***********");
                    properPubName = allRos[rosCount].getPublicationName();
                }
            }
        }

        //assign the proper publication name if it is available
        if (properPubName != "" && jorTargetRos != -1) {
            allRos[jorTargetRos].setCitedByCount(totalCitation);
        } else if (properPubName != "" && targetRos != -1) {
            allRos[targetRos].setPublicationName(properPubName);
        }

        if (isJournal == true && jorTargetRos != -1) {
            allRos[jorTargetRos].setCitedByCount(totalCitation);
            return allRos[jorTargetRos];
        } else if (targetRos != -1) {
            allRos[targetRos].setCitedByCount(totalCitation);
            return allRos[targetRos];
        } else {
            ROS emptyRos = new ROS();
            emptyRos.setArticleTitle(title);
            return emptyRos;
        }

    }

    /**
     * extracts the meta data returned from the scopus search method. uses the
     * multifield extraction method
     *
     * @param xmlString the xml string returned from the scopus and other
     * searches
     * @param title the title of the ros
     * @return
     * @throws Exception
     */
    public static ROS extractScopusMetadata(String xmlString, String title) throws Exception {

        int totalResults = 0;
        ROS refinedROS = new ROS();
        totalResults = Integer.parseInt(extractField("opensearch:totalResults", xmlString));
        if (totalResults == 0) {
            System.out.println("The search in Scopus returned no result. Trying other sources");
        } else if (totalResults > 0) {
            ROS[] ross = extractMultiField(xmlString);

            if (ross.length > 1) {
                refinedROS = extractBestROS(ross, title);
            } else {
                refinedROS = ross[0];
            }

        }
        return refinedROS;

    }

    /**
     *
     * @param xmlString
     * @param title
     * @return
     * @throws Exception
     */
    public static ROS extractNatureMetadata(String xmlString, String title) throws Exception {
        System.out.println("xml String from nature" + xmlString);
        ROS ros = new ROS();
        return ros;
    }

    /**
     *
     * @param xmlString
     * @param title
     * @return
     * @throws Exception
     */
    public static ROS extractCiteSeerMetadata(String xmlString, String title) throws Exception {
        //System.out.println("CiteSeer search returned:\n"+xmlString);
        Document doc = Jsoup.parse(xmlString);
        ROS ros = new ROS();
        String articleTitle = "";
        int pubYear = 0;

        Elements resultElements = doc.getElementsByClass("result");

        for (Element result : resultElements) {
            Elements titleElement = result.getElementsByClass("doc_details");
            System.out.println(titleElement.text() + "\t" + title);
            if (!titleElement.text().equalsIgnoreCase(title)) {
                break;
            } else {
                ros.setArticleTitle(title);
                Elements authorElement = result.getElementsByClass("pubinfo");
                //authors=authorElement.text();
                Elements yearElement = result.getElementsByClass("pubyear");
                String yearStr = yearElement.text().replace(", ", "");
                int year = (yearStr.length() > 0 ? Integer.parseInt(yearStr) : 0);
                System.out.println("year:" + yearElement.text().replace(", ", ""));
                ros.setYear(year);
                Elements citeElement = result.getElementsByClass("citation");
                String[] citedBy = citeElement.text().split(" ");

                int citeby = (citedBy.length > 2 ? Integer.parseInt(citedBy[2]) : -1);
                ros.setCitedByCount(citeby);

                Elements publicationElement = result.getElementsByClass("pubvenue");
                String pub = publicationElement.text().replace("- ", "").toLowerCase();
                if (pub.contains("journal")) {
                    ros.setPublicationType("Journal");
                } else if (pub.contains("conference") || pub.contains("conf.") || pub.contains("proc.")) {
                    ros.setPublicationType("Conference");
                } else {
                    ros.setPublicationType("");
                }
                ros.setPublicationName(pub);

            }

        }
        // }

        return ros;

    }

    /**
     * extracts the year string from the year field. Some times the xml tag
     * contains 03 or 2003 99 or 1999. This method extracts the year and if it
     * is >20 changes it to 20th otherwise to 21th century
     *
     * @param yearString
     * @return
     */
    private static int extractYearFromYearString(String yearString) {
        int year = 0;
        String ss[] = yearString.split(" ");
        for (String str : ss) {
            Matcher m = Pattern.compile("(19|20)\\d\\d|(dd)").matcher(str);
            if (m.find()) {
                year = Integer.parseInt(str);
            }
        }
        if (year > 1900) {
            return year;
        } else {
            if (year < 20) {
                return 2000 + year;
            } else {
                return 1900 + year;
            }
        }

    }

    /**
     * uses a regular expression to extract the year part from the date
     *
     * @param doi the doi extracted from the xml file
     * @return
     */
    private static String extractYearFromDOI(String doi) {
        String parts[] = doi.split("/");
        String yearString = parts[4];
        String year = "";
        String ss[] = yearString.split("\\.");
        for (String str : ss) {
            Matcher m = Pattern.compile("(19|20)\\d\\d").matcher(str);
            if (m.find()) {
                year = str;
            }
        }
        return year;
    }

    /**
     *
     * @param doi
     * @param ti
     * @param au
     * @param pubYear
     * @param xmlFile
     */
    public static void createROS(String doi, String ti, String au, String pubYear, String xmlFile) {
        try {
            DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder docBuilder = null;
            try {
                docBuilder = docFactory.newDocumentBuilder();

            } catch (ParserConfigurationException ex) {
                Logger.getLogger(MetadataExtractor.class
                        .getName()).log(Level.SEVERE, null, ex);
            }
            org.w3c.dom.Document rosDoc = docBuilder.newDocument();
            org.w3c.dom.Element rootElement = rosDoc.createElement("ROS");
            rosDoc.appendChild((rootElement));
            org.w3c.dom.Element DOI = rosDoc.createElement("DOI");
            org.w3c.dom.Element title = rosDoc.createElement("Title");
            org.w3c.dom.Element authors = rosDoc.createElement("Authors");
            org.w3c.dom.Element institution = rosDoc.createElement("Institution");
            org.w3c.dom.Element year = rosDoc.createElement("Year");
            org.w3c.dom.Element publication = rosDoc.createElement("Publication");
            org.w3c.dom.Element conference = rosDoc.createElement("Conference");

            rootElement.appendChild(DOI);
            rootElement.appendChild(title);
            rootElement.appendChild(authors);
            rootElement.appendChild(institution);
            rootElement.appendChild(year);
            rootElement.appendChild(publication);
            rootElement.appendChild(conference);

            DOI.appendChild(rosDoc.createTextNode(doi));
            title.appendChild(rosDoc.createTextNode(ti));
            authors.appendChild(rosDoc.createTextNode(au));
            year.appendChild(rosDoc.createTextNode(pubYear));

            TransformerFactory transformerFactory = TransformerFactory.newInstance();
            Transformer transformer = transformerFactory.newTransformer();
            DOMSource source = new DOMSource(rosDoc);
            xmlFile = xmlFile.replace(".xml", "MD.xml");
            System.out.println("Writing to:\n" + xmlFile);
            StreamResult result = new StreamResult(new File(xmlFile));
            transformer.transform(source, result);
            //result = new StreamResult(System.out);

        } catch (TransformerException ex) {
            Logger.getLogger(MetadataExtractor.class
                    .getName()).log(Level.SEVERE, null, ex);
        }

    }
}
