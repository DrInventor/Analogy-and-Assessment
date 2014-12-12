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

/**
 *
 * @author user
 */
public class SetupDatabase {

    private Connection connect = null;
  
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    private String user;
    private String password;
    private Configuration config=new Configuration();

    public SetupDatabase() throws Exception {
        try {
            Class.forName("com.mysql.jdbc.Driver");
            user = config.getDbUserName();
            password = config.getDbPassWord();
            //setup the connection with the DB should be hidden some where
            connect = DriverManager.getConnection("jdbc:mysql://localhost/", user, password);
            CreateDatabase();
            String ConferenceRankSql="CREATE Table IF NOT EXISTS ConferenceRank("
                    + "ID int NOT NULL AUTO_INCREMENT, "
                    + "conferenceRank int not null, "
                    + "shortName varchar(50), "
                    + "fullName varchar(200),"
                    + "publicationCount integer, "
                    + "fieldRating integer,"
                    + "citationRating integer, "
                    + "primary key(id));";
            CreateTable(ConferenceRankSql);
            
            String jcrSql="CREATE Table IF NOT EXISTS jcr("
                    + "ID int NOT NULL AUTO_INCREMENT, "
                    + "shortTitle varchar(200),"
                    + "issn varchar(20), "
                    + "totalCite int,"
                    + "impactFactor float,"
                    + "5yimpactFactor float,"
                    + "immediacyIndex float,"
                    + "articles int,"
                    + "citedHalfLife float,"
                    + "eigenFactorScore float,"
                    + "articleInfluence float,"
                    + "jcrYear int, primary key(id));";
            CreateTable(jcrSql);
            
            String journalAbbrSql="CREATE Table IF NOT EXISTS journalAbbr("
                    + "ID int NOT NULL AUTO_INCREMENT, "
                    + "fullName varchar(200),"
                    + "shortName varchar(60),"
                    + "primary key(id));";
            CreateTable(journalAbbrSql);
            
            String qsrankSql="CREATE Table IF NOT EXISTS qsRank("
                    + "ID int NOT NULL AUTO_INCREMENT, "
                    + "instRank int,"
                    + "instScore float,"
                    + "instName varchar(200),"
                    + "primary key(id));";
            CreateTable(qsrankSql);
            
            String scopusSql="CREATE Table IF NOT EXISTS scopus("
                    + "ID int NOT NULL AUTO_INCREMENT, "
                    + "doi varchar(100),"
                    + "articleTitle varchar(200),"
                    + "journalTitle varchar(200),"
                    + "conferenceTitle varchar(200),"
                    + "issn varchar(20),"
                    + "affiliation varchar(200),"
                    + "year int,"
                    + "citedByCount int,"
                    + "primary key(id));";
            CreateTable(scopusSql);
            
            String sjrSql="CREATE Table IF NOT EXISTS sjr("
                    + "ID int NOT NULL AUTO_INCREMENT,"
                    + "journaltitle varchar(200),"
                    + "issn varchar(20),"
                    + "sjr float,"
                    + "hIndex int,"
                    + "totaldocsCurYear int,"
                    + "totaldocs3years int,"
                    + "totalref int,"
                    + "totalCites3years int,"
                    + "citableDocs3years int,"
                    + "citesPerDoc float,"
                    + "refPerDoc float,"
                    + "sjrYear int,"
                    + "primary key(id));";
            CreateTable(sjrSql);

        } catch (ClassNotFoundException | SQLException e) {
            throw e;
        } finally {
            close();
        }
    }

    private void close() {
        try {
            resultSet.close();
            statement.close();
            connect.close();
        } catch (Exception e) {
        }
    }

    public int CreateDatabase() throws SQLException {
        connect = DriverManager.getConnection("jdbc:mysql://localhost/", user, password);
        String sql="CREATE DATABASE IF NOT EXISTS DRINVENTOR";
        try{
        statement = connect.createStatement();
        statement.executeUpdate(sql);
        
        }catch (SQLException se){
            System.out.println("Problem creating database...");
            se.printStackTrace();;
        } finally {
            close();
        }
        return 0;
    }
    public int CreateTable(String sql) throws SQLException {
        connect = DriverManager.getConnection("jdbc:mysql://localhost/drinventor", user, password);
               
        try{
        statement = connect.createStatement();
        statement.executeUpdate(sql);
        
        }catch (SQLException se){
            System.out.println("Problem creating table...");
            se.printStackTrace();;
        } finally {
            close();
        }
        return 0;
    }
}
