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
 * This class extract data from qs ranking and stores the data in a database table
 */
public class QSRank {

    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    private Configuration config= new Configuration();

   

    /**
     *
     * @throws Exception
     */
        public QSRank() throws Exception {
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
    public void extractInstRank() throws SQLException {
        //This code extracts the journal name and abbreviation of journals appearing on ISI
        //it sends the journal data to a mysql database and store it to journalabbr table with id, journal name and journal abbr fields
        String[] fields;

        Document doc = Jsoup.parse(getQS());
        System.out.println("Start of Body\n");
        System.out.println(doc.text());
        System.out.println("End of Body\n");
        Elements trElement = doc.getElementsByTag("tr");
        String[] instRank = new String[trElement.size()];
        String[] instScore = new String[trElement.size()];
        String[] instName = new String[trElement.size()];

        int instCount = 0;
        System.out.println("Extracted row size" + trElement.size());
        for (instCount = 0; instCount < 550; instCount++) {
            Element tr = trElement.get(instCount + 2);
            // System.out.println( "\nCurrnet row text=\n"+ tr.text());
            if (instCount < 399) {
                fields = tr.text().split(" ", 3);
                for (String st : fields) {
                   // System.out.println("the fields are "+ st);

                }

                instRank[instCount] = fields[0].trim();
                instScore[instCount] = fields[1].trim();
                instName[instCount] = fields[2].trim();
            } else {
                fields = tr.text().split(" ", 2);
                for (String st : fields) {
                //    System.out.println("the fields are "+ st);

                }

                instRank[instCount] = fields[0].substring(0, 3);
                float tempScore = (float) (33.2 - (Integer.parseInt(instRank[instCount]) - 400)/10 );
                instScore[instCount] = String.valueOf(tempScore);
                instName[instCount] = fields[1].trim();
            }
               // System.out.println(instRank[instCount] + " " + instScore[instCount] + " " + instName[instCount]);

        }       
            writeToQSRank(instRank, instScore, instName);
            System.out.println("Finished extracting QS Rank...");

    }

    /**
     *
     * @param qsRank    the qs rank of the institution
     * @param qsScore   the qs score of the institution
     * @param instName  the name of the institution
     * @return number  of records update/inserted
     * @throws SQLException
     */
    public int writeToQSRank(String[] qsRank, String[] qsScore, String[] instName) throws SQLException {

        try {
            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient
           
            for (int i = 0; i < 550; i++) {
                preparedStatement = connect.prepareStatement("insert into qsrank values(default, ?,?,?)");
                System.out.println("QSScore:" + i + qsScore[i]);
                System.out.println("QSName:" + instName[i]);
                preparedStatement.setInt(1, Integer.parseInt(qsRank[i]));
                preparedStatement.setFloat(2, Float.parseFloat(qsScore[i]));
                preparedStatement.setString(3, instName[i]);

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

    /**
     *
     * @return
     */
    public String getQS() {
        BufferedReader br = null;
        String sCurrentLine;
        String fullText = "";
        try {
            FileReader jrcFileReader = new FileReader("C:\\DRINVENTOR_PROJECT\\corpus\\qs.htm");
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

        return fullText;
    }
}
