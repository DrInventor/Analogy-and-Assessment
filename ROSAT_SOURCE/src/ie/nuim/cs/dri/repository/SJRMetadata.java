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
 * @author user
 */
public class SJRMetadata {

    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    Configuration config = new Configuration();

    public SJRMetadata() throws Exception {
        try {
            Class.forName("com.mysql.jdbc.Driver");

            //setup the connection with the DB should be hidden some where
            connect = DriverManager.getConnection("jdbc:mysql://localhost/" + config.getDbName() + "?"
                    + "user="+ config.getDbUserName()+"&password="+config.getDbPassWord());

        } catch (ClassNotFoundException | SQLException e) {
            throw e;
        } finally {
            close();
        }
    }

    public int writeToSJRDatabase(String[] jcrFields) throws SQLException {
        //prepare sql statement to insert the data passed via an array extracted from csv file
        try {
            statement = connect.createStatement();

            preparedStatement = connect.prepareStatement("insert into SJR values(default, ?,?,?,?,?,?,?,?,?,?,?,?)");
            preparedStatement.setString(1, jcrFields[1]);
            preparedStatement.setString(2, jcrFields[2]);
            preparedStatement.setFloat(3, Float.parseFloat(jcrFields[3]));
            preparedStatement.setInt(4, Integer.parseInt(jcrFields[4]));
            preparedStatement.setInt(5, Integer.parseInt(jcrFields[5]));
            preparedStatement.setInt(6, Integer.parseInt(jcrFields[6]));
            preparedStatement.setInt(7, Integer.parseInt(jcrFields[7]));
            preparedStatement.setInt(8, Integer.parseInt(jcrFields[8]));
            preparedStatement.setInt(9, Integer.parseInt(jcrFields[9]));
            preparedStatement.setFloat(10, Float.parseFloat(jcrFields[10]));
            preparedStatement.setFloat(11, Float.parseFloat(jcrFields[11]));
            preparedStatement.setInt(12, 2012);
            preparedStatement.executeUpdate();
            //for inspection uncomment here
            //System.out.println("Inspect sql statement here: "+preparedStatement.toString());
            
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }

    public void pareSJR() throws SQLException {
        //Read the csv file and parse it by its fields to send it to database or any where ba
        String sanitizedLine;
        String[] attributes;
        BufferedReader br = null;
        try {
            FileReader jrcFileReader = new FileReader(config.getCitationDataLocation() + "\\SJR2012.csv");
            br = new BufferedReader(jrcFileReader);
            String sCurrentLine;
            int records=0;
            //pushes the cursor to the second line avoiding the field header
            br.readLine();
            while ((sCurrentLine = br.readLine()) != null) {
                //sanitize removes unwanted characters specially , in a quotation
                sanitizedLine = sanitizeJCRData(sCurrentLine);
                attributes = sanitizedLine.split(",");
                writeToSJRDatabase(attributes);
                records++;
            }
            System.out.println(records +" Records Successfully Inserted");
        } catch (IOException e) {
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
            } catch (IOException ex) {
            }
        }

    }

    public String sanitizeJCRData(String sCurrentLine) {
        String nLine = "";
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
        return nLine;
    }

    public void searchJCR(String param, Object paramVal) throws Exception {
        try {

            statement = connect.createStatement();
            String query = "Select * from drinventormetadata.sjr where " + param + " = \'" + paramVal + "\';";
            //System.out.println(query);
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
        //System.out.println("Table: " + resultSet.getMetaData().getTableName(1));
        for (int i = 1; i <= resultSet.getMetaData().getColumnCount(); i++) {
            //System.out.println("Column " + i + " " + resultSet.getMetaData().getColumnName(i));
        }
        //System.out.println("==================================");
    }

    private void writeResultSet(ResultSet resultSet) throws SQLException {
        // resultSet is initialised before the first data set
        while (resultSet.next()) {
            // it is possible to get the columns via name
            // also possible to get the columns via the column number
            // which starts at 1
            // e.g., resultSet.getSTring(2);
            for (int i = 1; i < 12; i++) {
                //System.out.println(resultSet.getString(i));
            }
        }
       // System.out.println("==================================");
    }

    // you need to close all three to make sure
    private void close() {
        try {
            resultSet.close();
            statement.close();
            connect.close();
        } catch (Exception e) {
        }
    }
}
