/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

import java.util.Calendar;

/**
 *
 * @author Yalemisew
 * This class manages the Research Object Skeleton. It reads and writes ROS and 
 * provide support for other classes and methods.
 */
public class ROS {

    private int id;
    private String doi;
    private String articleTitle;
    private String publicationName;
    private String publicationType;
    private String issn;
    private String volume;
    private String affiliation1;
    private String affiliation2;
    private String affiliation3;
    private int year;
    private int citedByCount;
    private float journalIF;
    private int journalHI;
    private int conferenceRankingFR;
    private int conferenceRankingC;
    private int instRanking1;
    private int instRanking2;
    private int instRanking3;
    private String ROSinARFF;
    private String ROSinCSV;

    /**
     * Initializes all the parameters of the ROS
     */
    public ROS() {
        id = 0;
        doi = "";
        articleTitle = "";
        publicationName = "";
        publicationType = "";
        issn = "";
        volume = "";
        affiliation1 = "";
        affiliation2 = "";
        affiliation3 = "";
        year = 0;
        citedByCount = -1;
        journalIF = -1;
        journalHI = -1;
        conferenceRankingFR = -1;
        conferenceRankingC = -1;
        instRanking1 = 750;
        instRanking2 = 750;
        instRanking3 = 750;
        ROSinCSV = "DOI,Title,Publication Name,Publication Type,ISSN,Affiliation,Year,"
                + "Cited By Count,H Index,Impact Factor,Inst Rank,conference Citation Rating, conference Field Rating\n";
        ROSinARFF = "@relation ros \n\n@attribute publicationType real \n@attribute age real \n@attribute hIndex real"
                + "\n@attribute impactFactor real \n@attribute instRank real\n@attribute conferenceCitationRating real"
                + "\n@attribute conferenceFieldRating real  \n@attribute citedByCount real \n\n@data\n\n";

    }

    /**
     *
     * @return the id of the ROS
     */
    public int getId() {
        return id;
    }

    /**
     *
     * @param idVal the id value
     */
    public void setId(int idVal) {
        id = idVal;
    }

    /**
     *
     * @return the doi of the ROS
     */
    public String getDoi() {
        return doi;
    }

    /**
     *
     * @param doiVal the doi value of the ROS
     */
    public void setDoi(String doiVal) {
        doi = doiVal;
    }

    /**
     *
     * @return the title of the article (ROS)
     */
    public String getArticleTitle() {
        return articleTitle;
    }

    /**
     *
     * @param atVal the title of the ROS
     */
    public void setArticleTitle(String atVal) {
        articleTitle = atVal;
    }

    /**
     *
     * @return the publication name of the  ROS
     */
    public String getPublicationName() {
        return publicationName;
    }

    /**
     *
     * @param jtVal the publication of the ROS
     */
    public void setPublicationName(String jtVal) {
        publicationName = jtVal;
    }

    /**
     *
     * @return the type of the publication such as journal, conference proceeding etc
     */
    public String getPublicationType() {
        return publicationType;
    }

    /**
     *
     * @param ctVal the type of the publication
     */
    public void setPublicationType(String ctVal) {
        publicationType = ctVal;
    }

    /**
     *
     * @return the ISSN of the publication that contains the ROS
     */
    public String getIssn() {
        return issn;
    }

    /**
     *
     * @param issnVal the ISSN of the publication that contains the ROS
     */
    public void setIssn(String issnVal) {
        issn = issnVal;
    }

    /**
     *
     * @return The volume of the journal
     */
    public String getVolume() {
        return volume;
    }

    /**
     *
     * @param volumeVal the volume of the journal
     */
    public void setVolume(String volumeVal) {
        volume = volumeVal;
    }

    /**
     *
     * @return The affiliation of the authors of the ROS
     */
    public String getAffiliation1() {
        return affiliation1;
    }

    /**
     *
     * @param affVal1 The affiliation of the authors of the ROS
     */
    public void setAffiliation1(String affVal1) {
        affiliation1 = affVal1;
    }

    /**
     *
     * @return
     */
    public String getAffiliation2() {
        return affiliation2;
    }

    /**
     *
     * @param affVal2
     */
    public void setAffiliation2(String affVal2) {
        affiliation2 = affVal2;
    }

    /**
     *
     * @return
     */
    public String getAffiliation3() {
        return affiliation3;
    }

    /**
     *
     * @param affVal2
     */
    public void setAffiliation3(String affVal2) {
        affiliation3 = affVal2;
    }

    /**
     *
     * @return year of publication of the ROS
     */
    public int getYear() {
        return year;
    }

    /**
     *
     * @param yearVal year of publication of the ROS
     */
    public void setYear(int yearVal) {
        year = yearVal;
    }

    /**
     *
     * @return number of times the ROS is cited
     */
    public int getCitedByCount() {
        return citedByCount;
    }

    /**
     *
     * @param cbcVal number of times the ROS is cited
     */
    public void setCitedByCount(int cbcVal) {
        citedByCount = cbcVal;
    }

    /**
     *
     * @return the journal impact factor
     */
    public float getJournalIF() {
        return journalIF;
    }

    /**
     *
     * @param jifVal the journal impact factor
     */
    public void setJournalIF(float jifVal) {
        journalIF = jifVal;
    }

    /**
     *
     * @return the journal h-index
     */
    public float getJournalHI() {
        return journalHI;
    }

    /**
     *
     * @param jhiVal the journal h-index
     */
    public void setJournalHI(int jhiVal) {
        journalHI = jhiVal;
    }

    /**
     *
     * @return the conference rank, based on field rating
     */
    public float getConferenceRankingFR() {
        return conferenceRankingFR;
    }

    /**
     *
     * @param crfrVal the conference rank, based on field rating
     */
    public void setConferenceRankingFR(int crfrVal) {
        conferenceRankingFR = crfrVal;
    }

    /**
     *
     * @return the conference rank, based on citation rating
     */
    public float getConferenceRankingC() {
        return conferenceRankingC;
    }

    /**
     *
     * @param crcVal the conference rank, based on citation rating
     */
    public void setConferenceRankingC(int crcVal) {
        conferenceRankingC = crcVal;
    }

    /**
     *
     * @return the institution ranking, the first institution
     */
    public float getInstRanking1() {
        return instRanking1;
    }

    /**
     *
     * @param ir1Val the institution ranking, the first institution
     */
    public void setInstRanking1(int ir1Val) {
        instRanking1 = ir1Val;
    }

    /**
     *
     * @return
     */
    public float getInstRanking2() {
        return instRanking2;
    }

    /**
     *
     * @param ir2Val
     */
    public void setInstRanking2(int ir2Val) {
        instRanking2 = ir2Val;
    }

    /**
     *
     * @return
     */
    public float getInstRanking3() {
        return instRanking3;
    }

    /**
     *
     * @param ir3Val
     */
    public void setInstRanking3(int ir3Val) {
        instRanking3 = ir3Val;
    }

    /**
     * prepares the ros meta data to a csv file. each field of the ROS is
     * separated by comma, the name of the fields are set as a static field
     *
     * @param allROS an array of all the ROS in the repository
     * @return the data of the ROS in a CSV format
     * @throws java.lang.Exception
     */
    public String prepareForCSVFile(ROS[] allROS) throws Exception {
        for (ROS ros : allROS) {
            ROSinCSV += ros.getDoi() + "," + ros.getArticleTitle().replace(",", ":") + "," + ros.getPublicationName().replace(",", ":") + "," + ros.getPublicationType() + ","
                    + ros.getIssn() + "," + ros.getAffiliation1().replace(",", ":") + "," + ros.getYear() + "," + ros.getCitedByCount() + "," + ros.getJournalHI() + "," + ros.getJournalIF()
                    + "," + ros.getInstRanking1() + "," + ros.getConferenceRankingC() + "," + ros.getConferenceRankingFR() + "\n";
        }
        return ROSinCSV;
    }

    /**
     * prepares the ros meta data to a csv file. each field of the ROS is
     * separated by comma, the name of the fields are set as a static field
     *
     * @param allROS an array of all the ROS in the repository
     * @return the data of the ROS in arff format
     * @throws java.lang.Exception
     */
    public String prepareForARFFFile(ROS[] allROS) throws Exception {

        Object tempPubType, tempAge, tempHI, tempIF, tempIR1, tempCRF, tempCRC, tempCC;
        for (ROS ros : allROS) {
            if (ros.getPublicationType().equalsIgnoreCase("journal")) {
                tempPubType = "3";
            } else if (ros.getPublicationType().contains("Book Series")) {
                tempPubType = "2";
            } else if (ros.getPublicationType().contains("Conference Proceeding")) {

                tempPubType = "1";
            } else if (ros.getPublicationType().equals("")) {
                tempPubType = "?";
            } else {
                tempPubType = 0;
            }

            //age
            int year = Calendar.getInstance().get(Calendar.YEAR);

            if (ros.getYear() > 0) {
                tempAge = year - ros.getYear();
            } else {
                tempAge = "?";
            }

            tempHI = ((ros.getJournalHI() < 0) ? "?" : ros.getJournalHI());
            tempIF = ((ros.getJournalIF() < 0) ? "?" : ros.getJournalIF());
            tempIR1 = ((ros.getInstRanking1() <= 0) ? "?" : ros.getInstRanking1());
            tempCRC = ((ros.getConferenceRankingC() < 0) ? "?" : ros.getConferenceRankingC());
            tempCRF = ((ros.getConferenceRankingFR() < 0) ? "?" : ros.getConferenceRankingFR());
            tempCC = ((ros.getCitedByCount() < 0) ? "?" : ros.getCitedByCount());

            ROSinARFF += tempPubType + "," + tempAge + "," + tempHI + "," + tempIF + "," + tempIR1 + "," + tempCRC + "," + tempCRF + "," + tempCC + "\n";
        }
        return ROSinARFF;
    }

    /**
     * checks if the given ros is a journal. The logic here is, is the ros is
     * not identified as non journal but if it is found in a journal list when
     * we search its Impact factor then we treat the ros as a journal because
     * the publication is found in our journal list.
     */
    public void recheckPublicationType() {
        if (!this.publicationType.equalsIgnoreCase("journal") && this.journalIF > -1) {
            this.publicationType = "Journal";
        }
    }

    /**
     * Displays part of the data on a console
     */
    public void showMetaData() {
        System.out.println("=========================");
        System.out.println("ID: " + id);
        System.out.println("DOI: " + doi);
        System.out.println("articleTitle: " + articleTitle);
        System.out.println("publicationName: " + publicationName);
        System.out.println("publicationType: " + publicationType);
        System.out.println("issn: " + issn);
        System.out.println("affiliation1: " + affiliation1);
        System.out.println("affiliation2: " + affiliation2);
        System.out.println("affiliation3: " + affiliation3);
        System.out.println("year: " + year);
        System.out.println("journalIF: " + journalIF);
        System.out.println("journalHI: " + journalHI);
        System.out.println("conferenceRankingFR: " + conferenceRankingFR);
        System.out.println("instRanking1: " + instRanking1);
        System.out.println("=========================");
    }

    /**
     *Displays the full data on a console
     */
    public void showFullData() {
        System.out.println("=========================");
        System.out.println("ID: " + id);
        System.out.println("DOI: " + doi);
        System.out.println("articleTitle: " + articleTitle);
        System.out.println("publicationName: " + publicationName);
        System.out.println("publicationType: " + publicationType);
        System.out.println("issn: " + issn);
        System.out.println("affiliation1: " + affiliation1);
        System.out.println("affiliation2: " + affiliation2);
        System.out.println("affiliation3: " + affiliation3);
        System.out.println("year: " + year);
        System.out.println("citedByCount: " + citedByCount);
        System.out.println("journalIF: " + journalIF);
        System.out.println("journalHI: " + journalHI);
        System.out.println("conferenceRankingFR: " + conferenceRankingFR);
        System.out.println("conferenceRankingC: " + conferenceRankingC);
        System.out.println("instRanking1: " + instRanking1);
        System.out.println("instRanking2: " + instRanking2);
        System.out.println("instRanking3: " + instRanking3);
        System.out.println("=========================");
    }

}
