/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ie.nuim.cs.dri.repository;

import java.util.Comparator;

/**
 *
 * @author user
 */
public class TitleComparator  implements Comparator<ROS>
{
    public int compare(ROS o1, ROS o2)
    {
       
        return o1.getArticleTitle().compareTo(o2.getArticleTitle());
   }
}    

