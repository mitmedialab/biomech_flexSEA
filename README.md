//****************************************************************************  
//	**[Project]** FlexSEA: Flexible & Scalable Electronics Architecture  
//	**[Sub-project]** 'flexsea-manage' Mid-level computing, and networking  
//	*Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>*  
//	Licensed under the GNU General Public License v3.0 (GPL-3.0)  
//****************************************************************************  
//	**[Lead developer]** Jean-Francois (JF) Duval, jfduval at dephy.com.  
//	**[Origin]** Based on Jean-Francois Duval's work at the MIT Media Lab  
//	Biomechatronics research group <http://biomech.media.mit.edu/> 
//****************************************************************************

**Maintainer**: Tony Shu tonyshu@mit.edu

This repository is a direct fork from [Dephy's flexSEA rigid repository](https://github.com/DephyInc/fx-rigid-mn).
Branches **master**, **dev**, and **public** track the original.


# Cloning
New personal branches should clone from **public**!!! The following creates a copy of **public**, updates the submodules, and sets the remote to track your new branch.
```
$ git clone https://github.com/mitmedialab/biomech_flexSEA
$ cd biomech_flexSEA
$ git checkout public
$ git checkout -b yourInitials_yourBranchName
$ git submodule update --init
$ git push -u origin yourInitials_yourBranchName
```
# Schematics
Board schematics are located in the **master** branch. Feel free to incorporate the folder into your own branch for ease of access.
