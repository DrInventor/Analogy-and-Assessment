/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.metadata;

import java.awt.FileDialog;
import java.awt.Frame;
import java.io.File;
import java.util.HashSet;

/**
 *
 * @author Yalemisew
 */
public class DirectoryReader {

    private  HashSet<String> xmlFileList = new HashSet();

    /**
     *
     * @return xmlFileList the list of all xml files read in the folder
     */
    public HashSet<String> getXmlFileList(){
       return xmlFileList;
   }

    /**
     *
     * @param root the root director that contains all ROS in xml file
     */
    public  void readXMLFiles(String root) {
        File[] files = new File(root).listFiles();
        showFiles(files);

    }

    private  void showFiles(File[] files) {
        String directoryName = null;
        for (File file : files) {
            if (file.isDirectory()) {
                showFiles(file.listFiles()); // Calls same method again.
            } else {
                  if (file.getName().endsWith("xml")&& !file.getName().endsWith("MD.xml")) {
                    xmlFileList.add(file.getPath() );
                }

            }
        }

    }


}
