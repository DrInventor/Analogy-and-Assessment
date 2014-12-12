/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

/**
 *
 * @author Yalemisew
 * this class extracts computer science conference ranks and store the results
 * to a database table in mysql
 */
public class ConferenceRank {

    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;

    private String abbJournalTitle;
    private String journalTitle;
    private int publicationCount;
    private int FieldRating;
    private int citationRating;
     private Configuration config= new Configuration();

    public ConferenceRank() throws Exception {
        try {
            Class.forName("com.mysql.jdbc.Driver");

            //setup the connection with the DB should be hidden some where
            connect = DriverManager.getConnection("jdbc:mysql://localhost/" +config.getDbName()+"?"
                    + "user="+ config.getDbUserName()+"&password="+config.getDbPassWord());

        } catch (ClassNotFoundException | SQLException e) {
            throw e;
        } finally {
            close();
        }
    }
    public void extractConferenceRankCC() throws SQLException {
        //This code extracts the journal name and abbreviation of journals appearing on ISI
        //it sends the journal data to a mysql database and store it to journalabbr table with id, journal name and journal abbr fields
        String[] fields;
        for (int page =0 ; page < 36; page++) {
            try {
                String url = "http://academic.research.microsoft.com/RankList?entitytype=3&topDomainID=2&subDomainID=0&last=0&start=" + (page * 100 + 1) + "&end=" + ((page * 100) + 100)+"&orderBy=1";
                //http://academic.research.microsoft.com/RankList?entitytype=3&topDomainID=2&subDomainID=0&last=0&start=101&end=200&orderBy=1
                //Document doc = Jsoup.parse(getMAS());
                Document doc = Jsoup.connect(url)
                        .timeout(50000)
                        .get();
                //System.out.println("Body\n"+doc.body());
                //System.out.println("End of Body\n");
                Elements trElement = doc.getElementsByTag("tr");
                String[] conferenceRank = new String[trElement.size()];
                String[] conferenceShortName = new String[trElement.size()];
                String[] conferenceFullName = new String[trElement.size()];
                String[] conferencePubCount = new String[trElement.size()];
               // String[] conferenceFieldRating = new String[trElement.size()];
                String[] conferenceCitationRating = new String[trElement.size()];
                int confCount = 0;
                //System.out.println("Size=" + trElement.size());
                for (Element tr : trElement) {
                    // System.out.println("rows" + tr.text());
                    Elements tdElement = tr.getElementsByTag("td");
                    fields = new String[tdElement.size()];
                    int fieldCount = 0;
                    
                    if (tdElement.size() >= 4) {
                        // System.out.println("rows" + tr.text());
                        for (Element td : tdElement) {
                            fields[fieldCount] = td.text();
                            fieldCount++;
                            
                        }
                        conferenceRank[confCount] = fields[0].trim();
                        String[] titles = fields[1].split("-",2);
                        if (titles.length == 2) {
                            conferenceShortName[confCount] = titles[0].trim();
                            conferenceFullName[confCount] = titles[1].trim();
                        } else {
                            conferenceFullName[confCount] = fields[1].trim();
                            conferenceShortName[confCount] = "";
                        }
                        conferencePubCount[confCount] = fields[2].trim();
                        conferenceCitationRating[confCount] = fields[3].trim();
                       // System.out.println(conferenceRank[confCount]+" "+ conferenceShortName[confCount] + " " + conferenceFullName[confCount] + " " + conferencePubCount[confCount] + " " + conferenceCitationRating[confCount]);
                        confCount++;
                    }
                }
               // System.out.println("length"+conferenceRank.length);
                writeToConferenceCitationRating(conferenceRank, conferenceShortName, conferenceFullName, conferencePubCount, conferenceCitationRating);
                
            } catch (IOException ex) {
                Logger.getLogger(ConferenceRank.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
    public void extractConferenceRankFR() throws SQLException {
        //This code extracts the journal name and abbreviation of journals appearing on ISI
        //it sends the journal data to a mysql database and store it to journalabbr table with id, journal name and journal abbr fields
        String[] fields;
        for (int page =0 ; page < 36; page++) {
            try {
                String url = "http://academic.research.microsoft.com/RankList?entitytype=3&topDomainID=2&subDomainID=0&last=0&start=" + (page * 100 + 1) + "&end=" + ((page * 100) + 100);
                //String url= "http://academic.research.microsoft.com/RankList?entitytype=3&topDomainID=2&subDomainID=0&last=0&start=0&end=100";
               // http://academic.research.microsoft.com/RankList?entitytype=3&topdomainid=2&subdomainid=0&last=0&orderby=1
                //Document doc = Jsoup.parse(getMAS());
                Document doc = Jsoup.connect(url)
                        .timeout(50000)
                        .get();
              
                Elements trElement = doc.getElementsByTag("tr");
                String[] conferenceRank = new String[trElement.size()];
                String[] conferenceShortName = new String[trElement.size()];
                String[] conferenceFullName = new String[trElement.size()];
                String[] conferencePubCount = new String[trElement.size()];
                String[] conferenceFieldRating = new String[trElement.size()];
                //String[] conferenceCitationRating = new String[trElement.size()];
                int confCount = 0;
               // System.out.println("Size=" + trElement.size());
                for (Element tr : trElement) {
                    // System.out.println("rows" + tr.text());
                    Elements tdElement = tr.getElementsByTag("td");
                    fields = new String[tdElement.size()];
                    int fieldCount = 0;
                    
                    if (tdElement.size() >= 4) {
                        // System.out.println("rows" + tr.text());
                        for (Element td : tdElement) {
                            fields[fieldCount] = td.text();
                            fieldCount++;
                            
                        }
                        conferenceRank[confCount] = fields[0].trim();
                        String[] titles = fields[1].split(" - ",2);
                        if (titles.length == 2) {
                            conferenceShortName[confCount] = titles[0].trim();
                            conferenceFullName[confCount] = titles[1].trim();
                        } else {
                            conferenceFullName[confCount] = fields[1].trim();
                            conferenceShortName[confCount] = "";
                        }
                        conferencePubCount[confCount] = fields[2].trim();
                        conferenceFieldRating[confCount] = fields[3].trim();
                        System.out.println(conferenceRank[confCount]+" "+ conferenceShortName[confCount] + " " + conferenceFullName[confCount] + " " + conferencePubCount[confCount] + " " + conferenceFieldRating[confCount]);
                        confCount++;
                    }
                }
               // System.out.println("length"+conferenceRank.length);
                writeToConferenceRank(conferenceRank, conferenceShortName, conferenceFullName, conferencePubCount, conferenceFieldRating);
                
            } catch (IOException ex) {
                Logger.getLogger(ConferenceRank.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public int writeToConferenceRank(String[] conferenceRank, String[] conferenceShortName, String[] conferenceFullName, String[] conferencePubCount, String[] conferenceFieldRating) throws SQLException {

        try {
            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient
           // System.out.println("Length"+conferenceRank.length);
            for (int i = 0; i < conferenceRank.length-6; i++) {
                preparedStatement = connect.prepareStatement("insert into conferencerank values(default, ?,?,?,?,?,?)");
                preparedStatement.setInt(1, Integer.parseInt(conferenceRank[i]));
                preparedStatement.setString(2, conferenceShortName[i]);
                preparedStatement.setString(3, conferenceFullName[i]);
                preparedStatement.setInt(4, Integer.parseInt(conferencePubCount[i]));
                preparedStatement.setInt(5, Integer.parseInt(conferenceFieldRating[i]));
                preparedStatement.setInt(6, 0);
                preparedStatement.executeUpdate();
                //System.out.println("\n\n" + preparedStatement.toString());
            }
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }
     public int writeToConferenceCitationRating(String[] conferenceRank, String[] conferenceShortName, String[] conferenceFullName, String[] conferencePubCount, String[] conferenceCitationRating) throws SQLException {

        try {
            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient
            //System.out.println("Length"+conferenceRank.length);
            for (int i = 0; i < conferenceRank.length-6; i++) {
                preparedStatement = connect.prepareStatement("update conferencerank set citationRating=? where fullName=? and publicationCount=?");
                 preparedStatement.setInt(1, Integer.parseInt(conferenceCitationRating[i]));
                preparedStatement.setString(2, conferenceFullName[i]);
                preparedStatement.setInt(3, Integer.parseInt(conferencePubCount[i]));
               
                preparedStatement.executeUpdate();
                //System.out.println("\n\n" + preparedStatement.toString());
            }
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }


    private void close() {
        try {
            resultSet.close();
            statement.close();
            connect.close();
        } catch (Exception e) {
        }
    }

   
}
