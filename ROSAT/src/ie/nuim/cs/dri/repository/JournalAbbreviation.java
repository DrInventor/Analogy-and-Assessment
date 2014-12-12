/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

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
 * This class extracts journal abbreviations form the web and stores it to a
 * database table.
 */
public class JournalAbbreviation {
    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    private Configuration config= new Configuration();

    /**
     *
     * @throws Exception
     */
    public JournalAbbreviation() throws Exception {
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

    /**
     *
     * @throws SQLException
     */
    public void extractJournalTitle() throws SQLException {
        //This code extracts the journal name and abbreviation of journals appearing on ISI
        //it sends the journal data to a mysql database and store it to journalabbr table with id, journal name and journal abbr fields
       // System.out.println("Extracting data from the web and writing to database...");
        try {
            for (int letter = 0; letter < 26; letter++) {
                String url= "http://www.efm.leeds.ac.uk/~mark/ISIabbr/" + (char)(65+letter) +"_abrvjt.html";
                Document doc = Jsoup.connect(url).get();
                Elements aElement = doc.getElementsByTag("DT");
                String[] journalName = new String[aElement.size()];

                int i = 0;
                for (Element e : aElement) {
                    journalName[i] = e.text();
                    i++;
                }
                String[] journalAbbr = new String[aElement.size()];
                Elements DDElement = doc.getElementsByTag("DD");
                i = 0;
                for (Element e : DDElement) {
                    journalAbbr[i] = e.text();
                    i++;
                }
                writeToJournalTable(journalName, journalAbbr);
                for (i = 0; i < journalAbbr.length; i++) {
                System.out.println(journalName[i]+ "\t" + journalAbbr[i]);
                 }
            }
            System.out.println("Extracting journal Abbreviation completed...");
        } catch (IOException ex) {
            Logger.getLogger(JournalAbbreviation.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     *
     * @param journalName the name/ title of the journal
     * @param journalAbbr the abbreviation of the journal
     * @return
     * @throws SQLException
     */
    public int writeToJournalTable(String[] journalName, String[] journalAbbr) throws SQLException {

        try {
            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient
            for (int i = 0; i < journalName.length; i++) {
                preparedStatement = connect.prepareStatement("insert into journalAbbr values(default, ?,?)");
                preparedStatement.setString(1, journalName[i]);
                preparedStatement.setString(2, journalAbbr[i]);
                preparedStatement.executeUpdate();
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
