#include <mystdlib.h>
#include "meshing.hpp"



namespace netgen
{
  extern const char * tetrules[];
  // extern const char * tetrules2[];
  extern const char * prismrules2[];
  extern const char * pyramidrules[];
  extern const char * pyramidrules2[];


  extern double teterrpow; 






  // ====================================
  //
  // [RH]  main procedure for mesh generation for the mesh data type
  //
  //  
  MESHING3_RESULT MeshVolume (MeshingParameters & mp, Mesh& mesh3d, long threadID)
  {
#ifdef FSDEBUG
    std::cout << ".. in mesh volume .. " << std::endl; 
    std::cout << ".. num points: " << mesh3d.GetNP() << std::endl;       
    std::cout << ".. num elements: " << mesh3d.GetNE() << std::endl << std::endl;    
#endif
    
    int i, oldne;
    PointIndex pi;

    int meshed;
    int cntsteps; 


    ARRAY<INDEX_2> connectednodes;

    mesh3d.Compress();

    mesh3d.PrintMemInfo (cout);

    if (mp.checkoverlappingboundary)
      if (mesh3d.CheckOverlappingBoundary())
	throw NgException ("Stop meshing since boundary mesh is overlapping");

    int nonconsist = 0;
    
#ifdef FSDEBUG
    std::cout << "mesh3d.GetNDomains: " << mesh3d.GetNDomains() << std::endl;
#endif

    for (int k = 1; k <= mesh3d.GetNDomains(); k++)
      {
         //PrintMessage (3, "Check subdomain ", k, " / ", mesh3d.GetNDomains());
// 	std::cout << "  --- meshing process of subdomains: " << k << " of " << mesh3d.GetNDomains() << " domains.. " << std::endl;

	mesh3d.FindOpenElements(k);

	/*
	bool res = mesh3d.CheckOverlappingBoundary();
	if (res)
	  {
	    PrintError ("Surface is overlapping !!");
	    nonconsist = 1;
	  }
	*/

	bool res = mesh3d.CheckConsistentBoundary();
	if (res)
	  {
#ifdef FSDEBUG
	    std::cout << "Surface mesh not consistent" << std::endl;
#endif
 	    nonconsist = 1;
	  }
      }

#ifdef FSDEBUG
    std::cout << " .. after consistency check .. " << std::endl;
#endif

    if (nonconsist)
      {
	PrintError ("Stop meshing since surface mesh not consistent");
 	throw NgException ("Stop meshing since surface mesh not consistent");
      }

#ifdef FSDEBUG
    std::cout << ".. before access to mp.maxh .. " << std::endl;
#endif

    double globmaxh = mp.maxh;

    
    for (int k = 1; k <= mesh3d.GetNDomains(); k++)
      {
// 	if (multithread.terminate)
// 	  break;


	PrintMessage (2, "");

#ifdef FSDEBUG
        std::cout << "------  Meshing subdomain " <<  k <<  " of " << mesh3d.GetNDomains() << " .. in thread: " << threadID <<"  ------"<<std::endl;
#endif
 

	// [RH] local feature size
	//
	mp.maxh = min2 (globmaxh, mesh3d.MaxHDomain(k));

#ifdef FSDEBUG
	std::cout << "..before CalcSurfacesOfNode" << std::endl;
#endif

	mesh3d.CalcSurfacesOfNode();

#ifdef FSDEBUG
	std::cout << "..before FindOpenElements" << std::endl;
#endif

	mesh3d.FindOpenElements(k);

#ifdef FSDEBUG
	std::cout << "..after FindOpenElements" << std::endl;
#endif
	
	if (!mesh3d.GetNOpenElements())
	  continue;


	
	Box<3> domain_bbox(Point<3> (1e10, 1e10, 1e10),
			   Point<3> (-1e10, -1e10, -1e10));

	for (SurfaceElementIndex sei = 0; sei < mesh3d.GetNSE(); sei++)
	  {
	    const Element2d & el = mesh3d[sei];
	    if (el.IsDeleted() ) continue;
	    
	    if (mesh3d.GetFaceDescriptor(el.GetIndex()).DomainIn() == k ||
		mesh3d.GetFaceDescriptor(el.GetIndex()).DomainOut() == k)

	      for (int j = 0; j < el.GetNP(); j++)
		domain_bbox.Add (mesh3d[el[j]]);
	  }

        // [RH] BUG ?? why increase the domain bounding box ??
        // 
//	domain_bbox.Increase (0.01 * domain_bbox.Diam());

#ifdef FSDEBUG
	std::cout << "..checkpoint A" << std::endl;
#endif

	for (int qstep = 1; qstep <= 4; qstep++)
        {
           // std::cout << "qstep : " << qstep << std::endl;
	    if (mesh3d.HasOpenQuads())
	      {
#ifdef FSDEBUG
                 std::cout << "open quads : " << mesh3d.HasOpenQuads() << std::endl;
#endif

		string rulefile = ngdir;
                int increase = 0;

		const char ** rulep = NULL;
		switch (qstep)
		  {
		  case 1:
		    rulefile += "/rules/prisms2.rls";
		    rulep = prismrules2;
		    break;
		  case 2: // connect pyramid to triangle
		    rulefile += "/rules/pyramids2.rls";
		    rulep = pyramidrules2;
		    break;
		  case 3: // connect to vis-a-vis point
		    rulefile += "/rules/pyramids.rls";
		    rulep = pyramidrules;
		    break;
                  case 4: // increase search area
                    mp.increase_level=1;
                     break;
		  }
	      
		//		Meshing3 meshing(rulefile);
		Meshing3 meshing(rulep); 
	      
		MeshingParameters mpquad = mp;
	      
		mpquad.giveuptol = 15;
		mpquad.baseelnp = 4;
		mpquad.starshapeclass = 1000;
		mpquad.check_impossible =  qstep == 1;   // for prisms only (air domain in trafo)


		for (pi = PointIndex::BASE; 
		     pi < mesh3d.GetNP()+PointIndex::BASE; pi++)
		  meshing.AddPoint (mesh3d[pi], pi);

		mesh3d.GetIdentifications().GetPairs (0, connectednodes);
		for (i = 1; i <= connectednodes.Size(); i++)
		  meshing.AddConnectedPair (connectednodes.Get(i));
	      
		for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
		  {
		    Element2d hel = mesh3d.OpenElement(i);
		    meshing.AddBoundaryElement (hel);
		  }
	      
		oldne = mesh3d.GetNE();
	      
		meshing.GenerateMesh (mesh3d, mpquad);
	      
		for (i = oldne + 1; i <= mesh3d.GetNE(); i++)
		  mesh3d.VolumeElement(i).SetIndex (k);
#ifdef DEBUG	      
		std::cout << "mesh has " << mesh3d.GetNE() << " prism/pyramid elements" << endl;
#endif
	      
		mesh3d.FindOpenElements(k);
	      }
	  }

#ifdef FSDEBUG
	std::cout << "..checkpoint B" << std::endl;
#endif
 
	if (mesh3d.HasOpenQuads())
	  {
	    PrintSysError ("mesh has still open quads");
	    throw NgException ("Stop meshing since too many attempts");
	    // return MESHING3_GIVEUP;
	  }

#ifdef FSDEBUG
	std::cout << "..checkpoint C" << std::endl;
#endif

	if (mp.delaunay && mesh3d.GetNOpenElements())
	  {
	    Meshing3 meshing((const char**)NULL);
	 
	    mesh3d.FindOpenElements(k);
	  
	    for (pi = PointIndex::BASE; 
		 pi < mesh3d.GetNP()+PointIndex::BASE; pi++)
	      meshing.AddPoint (mesh3d[pi], pi);
	  
	    for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
	      meshing.AddBoundaryElement (mesh3d.OpenElement(i));

	    oldne = mesh3d.GetNE();

	    meshing.Delaunay (mesh3d, k, mp, threadID);

	    for (i = oldne + 1; i <= mesh3d.GetNE(); i++)
	      mesh3d.VolumeElement(i).SetIndex (k);

	    //PrintMessage (3, mesh3d.GetNP(), " points, ", mesh3d.GetNE(), " elements");
	  }

	cntsteps = 0;
	if (mesh3d.GetNOpenElements())
	do
	  {
	    if (multithread.terminate)
	      break;

	    mesh3d.FindOpenElements(k);

	    //PrintMessage (5, mesh3d.GetNOpenElements(), " open faces");
	    cntsteps++;

	    if (cntsteps > mp.maxoutersteps) 
	      throw NgException ("GSSE :: Stop meshing since too many attempts");

	    string rulefile = ngdir + "/tetra.rls";
	    //PrintMessage (1, "start tetmeshing");

	    //	  Meshing3 meshing(rulefile);
	    Meshing3 meshing(tetrules);
      
	    ARRAY<int, PointIndex::BASE> glob2loc(mesh3d.GetNP());
	    glob2loc = -1;

	    for (PointIndex pi = PointIndex::BASE; 
		 pi < mesh3d.GetNP()+PointIndex::BASE; pi++)
            {
	      if (domain_bbox.IsIn (mesh3d[pi]))
              {
		glob2loc[pi] = 
		  meshing.AddPoint (mesh3d[pi], pi);
              }
            }
	    for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
	      {
		Element2d hel = mesh3d.OpenElement(i);
		for (int j = 0; j < hel.GetNP(); j++)
		  hel[j] = glob2loc[hel[j]];
		meshing.AddBoundaryElement (hel);
	    // meshing.AddBoundaryElement (mesh3d.OpenElement(i));
	      }

	    oldne = mesh3d.GetNE();

	    mp.giveuptol = 15 + 10 * cntsteps; 
	    mp.sloppy = 5;
            

            if (cntsteps > 1)
            {
                    mp.increase_level+=1;
                    cntsteps = 0;

#ifdef FSDEBUG
                    std::cout << "  ---  increase the rule level to " << mp.increase_level << " ---      " << std::endl;
#endif

            }
	    meshing.GenerateMesh (mesh3d, mp);

	    for (ElementIndex ei = oldne; ei < mesh3d.GetNE(); ei++)
	      mesh3d[ei].SetIndex (k);
	  
	  
	    mesh3d.CalcSurfacesOfNode();
	    mesh3d.FindOpenElements(k);
	  
	    teterrpow = 2;
	    if (mesh3d.GetNOpenElements() != 0)
	      {
		meshed = 0;
		//PrintMessage (5, mesh3d.GetNOpenElements(), " open faces found");

		//	      mesh3d.Save ("tmp.vol");
/*                if ( cntsteps % 2 == 0 || cntsteps == 1)
                {
                   char tempn[10];
                   tempn[0] = 't';
                   tempn[1] = 'm';
                   tempn[2] = 'p';
                   tempn[3] = '0' + cntsteps / 10;
                   tempn[4] = '0' + cntsteps % 10;
                   tempn[5] = '.';
                   tempn[6] = 'v';
                   tempn[7] = 'o';
                   tempn[8] = 'l';
                   mesh3d.Save (tempn);
                }
*/

		MeshOptimize3d optmesh;

// 		const char * optstr = "mcmstmcmstmcmstmcm";
// 		//const char * optstr = "mcm";
// 		std::cout << " ## GSSE optimization .. :: " << optstr << std::endl;

// 		int j;
// 		for (j = 1; j <= strlen(optstr); j++)
// 		  {
// 		    mesh3d.CalcSurfacesOfNode();
// 		    mesh3d.FreeOpenElementsEnvironment(2);

// 		    switch (optstr[j-1])
// 		      {
// 		      case 'c': optmesh.CombineImprove(mesh3d, OPT_REST); break;
// 		      case 'd': optmesh.SplitImprove(mesh3d, OPT_REST); break;
// 		      case 's': optmesh.SwapImprove(mesh3d, OPT_REST); break;
// 		      case 't': optmesh.SwapImprove2(mesh3d, OPT_REST); break;
// 		      case 'm': mesh3d.ImproveMesh(OPT_REST); break;
// 		      }	  

// 		  }

		mesh3d.FindOpenElements(k);	      
		//PrintMessage (3, "Call remove problem");
		RemoveProblem (mesh3d, k);
		mesh3d.FindOpenElements(k);
	      }
	    else
	      {
		meshed = 1;
		// PrintMessage (1, "Success !");

#ifdef FSDEBUG
                std::cout << "## Volume meshing .. done..                                        " << std::endl;
#endif

	      }
	  }
	while (!meshed);
	
// 	std::cout <<  "  -- statistics: " << mesh3d.GetNP() << " points and " << mesh3d.GetNE() << " elements " << std::endl;
	//PrintMessage (1, mesh3d.GetNP(), " points, ",  mesh3d.GetNE(), " elements");

	    
      }

    mp.maxh = globmaxh;

    MeshQuality3d (mesh3d);

    return MESHING3_OK;
  }  




  /*


  MESHING3_RESULT MeshVolumeOld (MeshingParameters & mp, Mesh& mesh3d)
  {
  int i, k, oldne;


  int meshed;
  int cntsteps; 


  PlotStatistics3d * pstat;
  if (globflags.GetNumFlag("silentflag", 1) <= 2)
  pstat = new XPlotStatistics3d;
  else
  pstat = new TerminalPlotStatistics3d;

  cntsteps = 0;
  do
  {
  cntsteps++;
  if (cntsteps > mp.maxoutersteps) 
  {
  return MESHING3_OUTERSTEPSEXCEEDED;
  }


  int noldp = mesh3d.GetNP();
      
      
  if ( (cntsteps == 1) && globflags.GetDefineFlag ("delaunay"))
  {
  cntsteps ++;

  mesh3d.CalcSurfacesOfNode();


  for (k = 1; k <= mesh3d.GetNDomains(); k++)
  {
  Meshing3 meshing(NULL, pstat);

  mesh3d.FindOpenElements(k);
	      
  for (i = 1; i <= noldp; i++)
  meshing.AddPoint (mesh3d.Point(i), i);
	      
  for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
  {
  if (mesh3d.OpenElement(i).GetIndex() == k)
  meshing.AddBoundaryElement (mesh3d.OpenElement(i));
  }
	      
  oldne = mesh3d.GetNE();
  if (globflags.GetDefineFlag ("blockfill"))
  {
  if (!globflags.GetDefineFlag ("localh"))
  meshing.BlockFill 
  (mesh3d, mp.h * globflags.GetNumFlag ("relblockfillh", 1));
  else
  meshing.BlockFillLocalH (mesh3d);
  }
	      
  MeshingParameters mpd;
  meshing.Delaunay (mesh3d, mpd);

  for (i = oldne + 1; i <= mesh3d.GetNE(); i++)
  mesh3d.VolumeElement(i).SetIndex (k);
  }
  }

  noldp = mesh3d.GetNP();

  mesh3d.CalcSurfacesOfNode();
  mesh3d.FindOpenElements();
  for (k = 1; k <= mesh3d.GetNDomains(); k++)
  {
  Meshing3 meshing(globflags.GetStringFlag ("rules3d", NULL), pstat);
      
  Point3d pmin, pmax;
  mesh3d.GetBox (pmin, pmax, k);
	  
  rot.SetCenter (Center (pmin, pmax));

  for (i = 1; i <= noldp; i++)
  meshing.AddPoint (mesh3d.Point(i), i);

  for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
  {
  if (mesh3d.OpenElement(i).GetIndex() == k)
  meshing.AddBoundaryElement (mesh3d.OpenElement(i));
  }

  oldne = mesh3d.GetNE();


  if ( (cntsteps == 1) && globflags.GetDefineFlag ("blockfill"))
  {
  if (!globflags.GetDefineFlag ("localh"))
  {
  meshing.BlockFill 
  (mesh3d, 
  mp.h * globflags.GetNumFlag ("relblockfillh", 1));
  }
  else
  {
  meshing.BlockFillLocalH (mesh3d);
  }
  }


  mp.giveuptol = int(globflags.GetNumFlag ("giveuptol", 15));

  meshing.GenerateMesh (mesh3d, mp);

  for (i = oldne + 1; i <= mesh3d.GetNE(); i++)
  mesh3d.VolumeElement(i).SetIndex (k);
  }



  mesh3d.CalcSurfacesOfNode();
  mesh3d.FindOpenElements();
      
  teterrpow = 2;
  if (mesh3d.GetNOpenElements() != 0)
  {
  meshed = 0;
  std::cout << "Open elements found, old" << endl;
  const char * optstr = "mcmcmcmcm";
  int j;
  for (j = 1; j <= strlen(optstr); j++)
  switch (optstr[j-1])
  {
  case 'c': mesh3d.CombineImprove(); break;
  case 'd': mesh3d.SplitImprove(); break;
  case 's': mesh3d.SwapImprove(); break;
  case 'm': mesh3d.ImproveMesh(2); break;
  }	  
	  
  std::cout << "Call remove" << endl;
  RemoveProblem (mesh3d);
  std::cout << "Problem removed" << endl;
  }
  else
  meshed = 1;
  }
  while (!meshed);

  MeshQuality3d (mesh3d);

  return MESHING3_OK;
  }  

  */




  /*
  MESHING3_RESULT MeshMixedVolume(MeshingParameters & mp, Mesh& mesh3d)
  {
    int i, j;
    MESHING3_RESULT res;
    Point3d pmin, pmax;

    mp.giveuptol = 10;
    mp.baseelnp = 4;
    mp.starshapeclass = 100;

    //  TerminalPlotStatistics3d pstat;
  
    Meshing3 meshing1("pyramids.rls");
    for (i = 1; i <= mesh3d.GetNP(); i++)
      meshing1.AddPoint (mesh3d.Point(i), i);

    mesh3d.FindOpenElements();
    for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
      if (mesh3d.OpenElement(i).GetIndex() == 1)
	meshing1.AddBoundaryElement (mesh3d.OpenElement(i));

    res = meshing1.GenerateMesh (mesh3d, mp);

    mesh3d.GetBox (pmin, pmax);
    PrintMessage (1, "Mesh pyramids, res = ", res);
    if (res)
      exit (1);


    for (i = 1; i <= mesh3d.GetNE(); i++)
      mesh3d.VolumeElement(i).SetIndex (1);

    // do delaunay
  
    mp.baseelnp = 0;
    mp.starshapeclass = 5;

    Meshing3 meshing2(NULL);
    for (i = 1; i <= mesh3d.GetNP(); i++)
      meshing2.AddPoint (mesh3d.Point(i), i);
    
    mesh3d.FindOpenElements();
    for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
      if (mesh3d.OpenElement(i).GetIndex() == 1)
	meshing2.AddBoundaryElement (mesh3d.OpenElement(i));

    MeshingParameters mpd;
    meshing2.Delaunay (mesh3d, mpd);

    for (i = 1; i <= mesh3d.GetNE(); i++)
      mesh3d.VolumeElement(i).SetIndex (1);


    mp.baseelnp = 0;
    mp.giveuptol = 10;

    for (int trials = 1; trials <= 50; trials++)
      {
	if (multithread.terminate)
	  return MESHING3_TERMINATE;

	Meshing3 meshing3("tetra.rls");
	for (i = 1; i <= mesh3d.GetNP(); i++)
	  meshing3.AddPoint (mesh3d.Point(i), i);
      
	mesh3d.FindOpenElements();
	for (i = 1; i <= mesh3d.GetNOpenElements(); i++)
	  if (mesh3d.OpenElement(i).GetIndex() == 1)
	    meshing3.AddBoundaryElement (mesh3d.OpenElement(i));
      
	if (trials > 1)
	  CheckSurfaceMesh2 (mesh3d);
	res = meshing3.GenerateMesh (mesh3d, mp);
      
	for (i = 1; i <= mesh3d.GetNE(); i++)
	  mesh3d.VolumeElement(i).SetIndex (1);

	if (res == 0) break;



	for (i = 1; i <= mesh3d.GetNE(); i++)
	  {
	    const Element & el = mesh3d.VolumeElement(i);
	    if (el.GetNP() != 4)
	      {
		for (j = 1; j <= el.GetNP(); j++)
		  mesh3d.AddLockedPoint (el.PNum(j));
	      }
	  }

	mesh3d.CalcSurfacesOfNode();
	mesh3d.FindOpenElements();

	MeshOptimize3d optmesh;

	teterrpow = 2;
	const char * optstr = "mcmcmcmcm";
	for (j = 1; j <= strlen(optstr); j++)
	  switch (optstr[j-1])
	    {
	    case 'c': optmesh.CombineImprove(mesh3d, OPT_REST); break;
	    case 'd': optmesh.SplitImprove(mesh3d); break;
	    case 's': optmesh.SwapImprove(mesh3d); break;
	    case 'm': mesh3d.ImproveMesh(); break;
	    }	  
	        
	RemoveProblem (mesh3d);
      }


    PrintMessage (1, "Meshing tets, res = ", res);
    if (res)
      {
	mesh3d.FindOpenElements();
	PrintSysError (1, "Open elemetns: ", mesh3d.GetNOpenElements());
	exit (1);
      }


  
    for (i = 1; i <= mesh3d.GetNE(); i++)
      {
	const Element & el = mesh3d.VolumeElement(i);
	if (el.GetNP() != 4)
	  {
	    for (j = 1; j <= el.GetNP(); j++)
	      mesh3d.AddLockedPoint (el.PNum(j));
	  }
      }
  
    mesh3d.CalcSurfacesOfNode();
    mesh3d.FindOpenElements();
  
    MeshOptimize3d optmesh;

    teterrpow = 2;
    const char * optstr = "mcmcmcmcm";
    for (j = 1; j <= strlen(optstr); j++)
      switch (optstr[j-1])
	{
	case 'c': optmesh.CombineImprove(mesh3d, OPT_REST); break;
	case 'd': optmesh.SplitImprove(mesh3d); break;
	case 's': optmesh.SwapImprove(mesh3d); break;
	case 'm': mesh3d.ImproveMesh(); break;
	}	  


    return MESHING3_OK;
  }
*/






  MESHING3_RESULT OptimizeVolume (MeshingParameters & mp, 
				  Mesh & mesh3d)
    //				  const CSGeometry * geometry)
  {
    int i, j;

    PrintMessage (1, "Volume Optimization");

    /*
      if (!mesh3d.PureTetMesh())
      return MESHING3_OK;
    */

    // std::cout << "optstring = " << mp.optimize3d << endl;
    /*
      const char * optstr = globflags.GetStringFlag ("optimize3d", "cmh");
      int optsteps = int (globflags.GetNumFlag ("optsteps3d", 2));
    */

    mesh3d.CalcSurfacesOfNode();
    for (i = 1; i <= mp.optsteps3d; i++)
      {
	if (multithread.terminate)
	  break;

/*	MeshOptimize3d optmesh;

	teterrpow = mp.opterrpow;
	for (j = 1; j <= strlen(mp.optimize3d); j++)
	  {
	    if (multithread.terminate)
	      break;

	    switch (mp.optimize3d[j-1])
	      {
	      case 'c': optmesh.CombineImprove(mesh3d, OPT_REST); break;
	      case 'd': optmesh.SplitImprove(mesh3d); break;
	      case 's': optmesh.SwapImprove(mesh3d); break;
	      case 't': optmesh.SwapImprove2(mesh3d); break;
#ifdef SOLIDGEOM
	      case 'm': mesh3d.ImproveMesh(*geometry); break;
	      case 'M': mesh3d.ImproveMesh(*geometry); break;
#else
	      case 'm': mesh3d.ImproveMesh(); break;
	      case 'M': mesh3d.ImproveMesh(); break;
#endif
	      
	      case 'j': mesh3d.ImproveMeshJacobian(); break;
	      }
	  }
	// [RH] .. ?? necessary
	//
        */
	MeshQuality3d (mesh3d);

      }
  
    return MESHING3_OK;
  }




  void RemoveIllegalElements (Mesh & mesh3d)
  {
    int it = 10;
    int nillegal, oldn;
    int i;

    PrintMessage (1, "Remove Illegal Elements");
    // return, if non-pure tet-mesh
    /*
      if (!mesh3d.PureTetMesh())
      return;
    */
    mesh3d.CalcSurfacesOfNode();

    nillegal = mesh3d.MarkIllegalElements();

    MeshOptimize3d optmesh;
    while (nillegal && (it--) > 0)
      {
	if (multithread.terminate)
	  break;

	PrintMessage (5, nillegal, " illegal tets");
	optmesh.SplitImprove (mesh3d, OPT_LEGAL);

	mesh3d.MarkIllegalElements();  // test
	optmesh.SwapImprove (mesh3d, OPT_LEGAL);
	mesh3d.MarkIllegalElements();  // test
	optmesh.SwapImprove2 (mesh3d, OPT_LEGAL);

	oldn = nillegal;
	nillegal = mesh3d.MarkIllegalElements();

	if (oldn != nillegal)
	  it = 10;
      }
    PrintMessage (5, nillegal, " illegal tets");
  }
}
