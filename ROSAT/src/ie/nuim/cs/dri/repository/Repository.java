/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

/**
 *
 * @author user
 */
public class Repository {

    public static void main(String[] args) throws Exception {
        System.out.println("Setting up the required resources and data...");
        System.out.println("=======================================================\n");
        SetupDatabase sdb = new SetupDatabase();
        System.out.println("\nPopulating Journal Citation Report Data from a file...");
        System.out.println("=======================================================\n");
        JCRMetadata jcrm = new JCRMetadata();
        jcrm.parseJCR();
        //jcrm.searchJCR("abbJournalTitle", "World Wide Web");
        System.out.println("\nPopulating Journal Abbreviations from the web...");
        System.out.println("=======================================================\n");
        JournalAbbreviation ja = new JournalAbbreviation();
        ja.extractJournalTitle();

        System.out.println("\nPopulating Journal Abbreviations...");
        SJRMetadata sjrm = new SJRMetadata();
        sjrm.pareSJR();
        System.out.println("\nPopulating Conference Rank from MAS...");
        
        ConferenceRank cr = new ConferenceRank();
        cr.extractConferenceRankFR();
        System.out.println("\nPopulating Conference Rank from MAS...this will take a while");
        cr.extractConferenceRankCC();

        QSRank qr= new QSRank();
        qr.extractInstRank();
        System.out.println("The Repository is set successfully!");
    }

}
