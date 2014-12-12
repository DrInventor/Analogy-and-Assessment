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

/**
 *
 * @author Yalemisew
 * This class parse the downloaded JCR file and stores the results in to database
 * table. A sample of the JCR file is found under citation data folder in this project
 */
public class JCRMetadata {

    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    Configuration config= new Configuration();

    /**
     *
     * @throws Exception
     */
    public JCRMetadata() throws Exception {
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
     * @param jcrFields
     * @return number of rows updated in the database table
     * @throws SQLException
     */
    public int writeToJCRDatabase(String[] jcrFields) throws SQLException {

        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;
            preparedStatement = connect.prepareStatement("insert into jcr values(default, ?,?,?,?,?,?,?,?,?,?,?)");
            preparedStatement.setString(1, jcrFields[0]);
            preparedStatement.setString(2, jcrFields[1]);
            preparedStatement.setInt(3, Integer.parseInt(jcrFields[2]));
            preparedStatement.setFloat(4, Float.parseFloat(jcrFields[3]));
            preparedStatement.setFloat(5, Float.parseFloat(jcrFields[4]));
            preparedStatement.setFloat(6, Float.parseFloat(jcrFields[5]));
            preparedStatement.setFloat(7, Integer.parseInt(jcrFields[6]));
            preparedStatement.setFloat(8, Float.parseFloat(jcrFields[7]));
            preparedStatement.setFloat(9, Float.parseFloat(jcrFields[8]));
            preparedStatement.setFloat(10, Float.parseFloat(jcrFields[9]));
            preparedStatement.setFloat(11, 2014);

            preparedStatement.executeUpdate();

        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }

    /**
     *
     * @throws SQLException
     */
    public void parseJCR() throws SQLException {
        BufferedReader br = null;
        try {
            FileReader jrcFileReader = new FileReader(config.getCitationDataLocation()+"\\jcr_recs.txt");
            br = new BufferedReader(jrcFileReader);
            String sCurrentLine;
            br.readLine();
            br.readLine();
            br.readLine();
            String[] attributes = new String[10];
            while ((sCurrentLine = br.readLine()) != null) {
                System.out.println(sCurrentLine.toString());
                sCurrentLine = sCurrentLine.replace(";;", ";0;");
                sCurrentLine = sCurrentLine.replace(">", "");
                
                attributes = sCurrentLine.split(";");
                for (int i = 0; i < attributes.length; i++) {
                    if (attributes[i].isEmpty()) {
                        attributes[i] = "0";
                    }
                    // System.out.println(attributes[i] + "\t");

                }
               
                int ans = writeToJCRDatabase(attributes);
                if (ans > 0) {
                   // System.out.println("Successfully inserted");
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }

    }

    /**
     *
     * @param field the column of the JCR table
     * @param paramVal  the value of the column searched
     * @throws Exception
     */
    public void searchJCR(String field, Object paramVal) throws Exception {
        try {

            statement = connect.createStatement();
            String query = "Select * from drinventormetadata.jcr where " + field+ " = \'" + paramVal + "\';";
           // System.out.println(query);
            resultSet = statement.executeQuery(query);
            writeResultSet(resultSet);

        } catch (Exception e) {
            throw e;
        } finally {
            close();
        }
    }

    private void writeMetaData(ResultSet resultSet) throws SQLException {
        // now get some metadata from the database
       // System.out.println("The columns in the table are: ");
       // System.out.println("Table: " + resultSet.getMetaData().getTableName(1));
        for (int i = 1; i <= resultSet.getMetaData().getColumnCount(); i++) {
           // System.out.println("Column " + i + " " + resultSet.getMetaData().getColumnName(i));
        }
       // System.out.println("==================================");
    }

    private void writeResultSet(ResultSet resultSet) throws SQLException {
        // resultSet is initialised before the first data set
        while (resultSet.next()) {
           
            for (int i = 1; i < 12; i++) {
                System.out.println(resultSet.getString(i));
            }
        }
        System.out.println("==================================");
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
     * @param sCurrentLine the current record to be sanitized and  inserted
     * @return
     */
    public String sanitizeJCRData(String sCurrentLine) {
        String nLine = "";
        //System.out.println("==============before=============");
        // System.out.println(sCurrentLine);
        sCurrentLine = sCurrentLine.replace(",,", ",0,");
        boolean inQuote = false;
        nLine = "";
        for (int i = 0; i < sCurrentLine.length(); i++) {
            if (inQuote == false && sCurrentLine.charAt(i) == '\"') {
                inQuote = true;
                //System.out.println("1 AT " + i + " " + sCurrentLine.charAt(i) + " in quotation" + inQuote );
            } else if (inQuote == true && sCurrentLine.charAt(i) == ',') {

                //do nothing 
            } else if (inQuote == true && sCurrentLine.charAt(i) == '\"') {
                inQuote = false;
                //System.out.println("1 AT " + i + " " + sCurrentLine.charAt(i) + " in quotation" + inQuote );
            } else {
                nLine += sCurrentLine.charAt(i);
            }

        }
       // System.out.println("==============After=============");
       //System.out.println(nLine);
        return nLine;
    }
}
