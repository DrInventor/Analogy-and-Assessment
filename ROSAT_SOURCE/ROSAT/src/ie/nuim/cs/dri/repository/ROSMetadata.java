/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;


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
public class ROSMetadata {

    private Connection connect = null;
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;

    /**
     *
     * @throws Exception
     */
    public ROSMetadata() throws Exception {
        try {
            Class.forName("com.mysql.jdbc.Driver");
            Configuration config= new Configuration();
            
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
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int writeToScopusDatabase(String[] scopusFields) throws SQLException {

        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;
            preparedStatement = connect.prepareStatement("insert into drinventormetadata.scopus values(default, ?,?,?,?,?,?,?,?)");
            preparedStatement.setString(1, scopusFields[0]);
            preparedStatement.setString(2, scopusFields[1]);
            preparedStatement.setString(3, scopusFields[2]);
            preparedStatement.setString(4, scopusFields[3]);
            preparedStatement.setString(5, scopusFields[4]);
            preparedStatement.setString(6, scopusFields[5]);
            preparedStatement.setInt(7, Integer.parseInt(scopusFields[6]));
            preparedStatement.setInt(8, Integer.parseInt(scopusFields[7].trim()));

            preparedStatement.executeUpdate();

        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int writeToScopusDatabase(ROS scopusFields) throws SQLException {

        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;
            preparedStatement = connect.prepareStatement("insert into drinventormetadata.scopus values(default, ?,?,?,?,?,?,?,?)");
            preparedStatement.setString(1, scopusFields.getDoi());
            preparedStatement.setString(2, scopusFields.getArticleTitle());
            preparedStatement.setString(3, scopusFields.getPublicationName());
            preparedStatement.setString(4, scopusFields.getPublicationType());
            preparedStatement.setString(5, scopusFields.getIssn());
            preparedStatement.setString(6, scopusFields.getAffiliation1());
            preparedStatement.setInt(7, scopusFields.getYear());
            preparedStatement.setInt(8, scopusFields.getCitedByCount());

            preparedStatement.executeUpdate();

        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return preparedStatement.getUpdateCount();
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public float getImpactFactor(ROS scopusFields) throws SQLException {
        float impFactor = -1;
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;

            String stm = "Select impactfactor from jcr where shortTitle in (select shortName from journalabbr where fullName=?)";
            preparedStatement = connect.prepareStatement(stm);
            preparedStatement.setString(1, scopusFields.getPublicationName());
            //System.out.println(stm);
            resultSet = preparedStatement.executeQuery();
            while (resultSet.next()) {
                impFactor = resultSet.getFloat("impactfactor");

            }

        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return impFactor;
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int getHindex(ROS scopusFields) throws SQLException {
        int hindex = -1;
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;
            String stm = "Select hindex from sjr where journaltitle=?";
            preparedStatement = connect.prepareStatement(stm);       
            preparedStatement.setString(1, scopusFields.getPublicationName());
            //System.out.println(stm);
            resultSet = preparedStatement.executeQuery();
            while (resultSet.next()) {
                hindex = resultSet.getInt("hindex");
            }

        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return hindex;
    }
    
    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int getInstitutionRank(ROS scopusFields) throws SQLException {
        int instRank = 750;
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;

            String stm = "Select instRank from qsRank where instName like ?";
            preparedStatement = connect.prepareStatement(stm);
            if (!scopusFields.getAffiliation1().isEmpty())
                preparedStatement.setString(1, "%"+scopusFields.getAffiliation1()+"%");
            else
                preparedStatement.setString(1, scopusFields.getAffiliation1());
            resultSet = preparedStatement.executeQuery();
            while (resultSet.next()) {
                instRank = resultSet.getInt("instRank");
            }
            if(instRank==750)
                instRank=getInstitutionRank2(scopusFields);
            // System.out.println(preparedStatement.toString());
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return instRank;
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int getInstitutionRank2(ROS scopusFields) throws SQLException {
        int instRank = 750;
        StringMatch strMatch= new StringMatch();
        int minRank=750, minScore=8;
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;

            String stm = "Select instRank, instName from qsRank";
            preparedStatement = connect.prepareStatement(stm);
            //preparedStatement.setString(1, scopusFields.getAffiliation1());
            resultSet = preparedStatement.executeQuery();
            int simScore;
            while (resultSet.next()) {
                String instName=resultSet.getString("instName");
                String instName2=scopusFields.getAffiliation1();
                simScore=strMatch.computeLevenshteinDistance(instName, instName2);
                if(simScore<=minScore){
                    minScore=simScore;
                    minRank=resultSet.getInt("instRank");
                }
                instRank = resultSet.getInt("instRank");
                
            }
        //System.out.println(preparedStatement.toString());
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        //System.out.println( minScore+"\t\t"+minRank+"\t"+scopusFields.getAffiliation1());
                
        return minRank;
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int[] getConferenceRank(ROS scopusFields) throws SQLException {
        int fieldRating = -1;
        int citationRating=-1;
        int[] confRating= new int[2];
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;

            String stm = "Select fieldrating, citationrating from conferenceRank where fullname like ?";
            preparedStatement = connect.prepareStatement(stm);
            if (!scopusFields.getAffiliation1().isEmpty())
                preparedStatement.setString(1, "%"+scopusFields.getPublicationName()+"%");
            else
                preparedStatement.setString(1, scopusFields.getPublicationName());
            resultSet = preparedStatement.executeQuery();
            while (resultSet.next()) {
                fieldRating = resultSet.getInt("fieldRating");
                citationRating=resultSet.getInt("citationRating");
            }
            confRating[0]=fieldRating;
            confRating[1]=citationRating;
            if(citationRating==0 && fieldRating==0 && !scopusFields.getAffiliation1().isEmpty())
                confRating=getConferenceRank2(scopusFields);
        //System.out.println(preparedStatement.toString());
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
        return confRating;
    }
    
    /**
     *
     * @return
     * @throws SQLException
     */
    public String dumpConference() throws SQLException {
        String fullName="";
        int citationRating=-1;
        int[] confRating= new int[2];
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;

            String stm = "Select fullname from  conferenceRank";
            preparedStatement = connect.prepareStatement(stm);
            resultSet = preparedStatement.executeQuery();
            System.out.println(resultSet.getFetchSize()+" conferences found");
            while (resultSet.next()) {
               // fullName +=  resultSet.getString("fullname")+"\n";
                 fullName +=  "\""+resultSet.getString("fullname")+"\""+"\n";
            }
          System.out.println(fullName);
        //System.out.println(preparedStatement.toString());
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
      return fullName;
    }

    /**
     *
     * @param scopusFields
     * @return
     * @throws SQLException
     */
    public int[] getConferenceRank2(ROS scopusFields) throws SQLException {
         int fieldRating = -1;
        int citationRating=-1;
        int[] confRating= new int[2];
        StringMatch strMatch= new StringMatch();
        int minRank=750, minScore=8;
        try {

            statement = connect.createStatement();
            //prepared statements can use variables and are more efficient;


            String stm = "Select fieldRating, citationRating, fullname from conferenceRank";
            preparedStatement = connect.prepareStatement(stm);
            //preparedStatement.setString(1, scopusFields.getAffiliation1());
            resultSet = preparedStatement.executeQuery();
            int simScore;
            while (resultSet.next()) {
                String confName=resultSet.getString("fullname");
                String confName2=scopusFields.getPublicationName();
                simScore=strMatch.computeLevenshteinDistance(confName, confName2);
                if(simScore<=minScore){
                    minScore=simScore;
                    citationRating=resultSet.getInt("citationRating");
                    fieldRating=resultSet.getInt("fieldRating");
                }
                 confRating[0]=fieldRating;
                 confRating[1]=citationRating;
            }
       // System.out.println(preparedStatement.toString());
        } catch (SQLException ex) {
            Logger.getLogger(JCRMetadata.class.getName()).log(Level.SEVERE, null, ex);
        }
       // System.out.println( minScore+"\t\t"+minRank+"\t"+scopusFields.getAffiliation1());
                
        return confRating;
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
