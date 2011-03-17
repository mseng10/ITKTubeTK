/*=========================================================================

Library:   TubeTK

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/
#ifndef __itkAnisotropicDiffusiveSparseRegistrationFilter_h
#define __itkAnisotropicDiffusiveSparseRegistrationFilter_h

#include "itkDiffusiveRegistrationFilter.h"

#include "itkGroupSpatialObject.h"
#include "itkVesselTubeSpatialObject.h"
#include "vtkSmartPointer.h"
class vtkFloatArray;
class vtkPointLocator;
class vtkPolyData;

namespace itk
{

/** \class itkAnisotropicDiffusiveSparseRegistrationFilter
 * \brief Registration filter for registrations using anisotropic diffusive
 * regularizers, for example for sliding organ registration.
 *
 * Traditional deformable image registration imposes a uniform
 * smoothness constraint on the deformation field. This
 * is not appropriate when registering images visualizing organs
 * that slide relative to each other, and therefore leads to registration
 * inaccuracies.
 *
 * This filter includes a regularization term based on anisotropic diffusion
 * that accomodates deformation field discontinuities that are expected when
 * considering sliding motion.
 *
 * TODO
 *
 * See: D.F. Pace et al., Deformable image registration of sliding organs using
 * anisotropic diffusive regularization, ISBI 2011.
 *
 * This class is templated over the type of the fixed image, the type of the
 * moving image and the type of the deformation field.
 *
 * \sa itkDiffusiveRegistrationFilter
 * \sa itkAnisotropicDiffusiveRegistrationFilter
 * \sa itkAnisotropicDiffusiveRegistrationFunction
 * \ingroup DeformableImageRegistration
 * \ingroup MultiThreaded
 */

template < class TFixedImage, class TMovingImage, class TDeformationField >
class ITK_EXPORT AnisotropicDiffusiveSparseRegistrationFilter
  : public DiffusiveRegistrationFilter< TFixedImage,
                                        TMovingImage,
                                        TDeformationField >
{
public:
  /** Standard class typedefs. */
  typedef AnisotropicDiffusiveSparseRegistrationFilter      Self;
  typedef DiffusiveRegistrationFilter< TFixedImage,
                                       TMovingImage,
                                       TDeformationField >  Superclass;
  typedef SmartPointer< Self >                              Pointer;
  typedef SmartPointer< const Self >                        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Self, DiffusiveRegistrationFilter);

  /** Inherit some parameters from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  /** Convenient typedefs from the superclass. */
  typedef typename Superclass::FixedImageType           FixedImageType;
  typedef typename Superclass::FixedImagePointer        FixedImagePointer;
  typedef typename Superclass::MovingImageType          MovingImageType;
  typedef typename Superclass::MovingImagePointer       MovingImagePointer;
  typedef typename Superclass::DeformationFieldType     DeformationFieldType;
  typedef typename Superclass::DeformationFieldPointer  DeformationFieldPointer;
  typedef typename Superclass::TimeStepType             TimeStepType;
  typedef typename Superclass::FiniteDifferenceFunctionType
      FiniteDifferenceFunctionType;
  typedef typename Superclass::OutputImageType          OutputImageType;
  typedef typename Superclass::OutputImagePointer       OutputImagePointer;
  typedef typename Superclass::OutputImageRegionType    OutputImageRegionType;

  /** The registration function type */
  typedef typename Superclass::RegistrationFunctionType
      RegistrationFunctionType;
  typedef typename Superclass::SpacingType              SpacingType;

  /** Deformation field types. */
  typedef typename Superclass::DeformationVectorType    DeformationVectorType;
  typedef typename Superclass::DeformationVectorComponentType
      DeformationVectorComponentType;
  typedef typename Superclass::DeformationComponentImageArrayType
      DeformationComponentImageArrayType;

  /** Diffusion tensor image types */
  typedef typename Superclass::DiffusionTensorType      DiffusionTensorType;
  typedef typename Superclass::DiffusionTensorImageType
      DiffusionTensorImageType;

  /** Scalar derivative image types */
  typedef typename Superclass::ScalarDerivativeImageType
      ScalarDerivativeImageType;
  typedef typename Superclass::ScalarDerivativeImagePointer
      ScalarDerivativeImagePointer;
  typedef typename Superclass::ScalarDerivativeImageArrayType
      ScalarDerivativeImageArrayType;

  /** Tensor derivative matrix image types */
  typedef typename Superclass::TensorDerivativeImageType
      TensorDerivativeImageType;
  typedef typename Superclass::TensorDerivativeImagePointer
      TensorDerivativeImagePointer;
  typedef typename Superclass::TensorDerivativeImageArrayType
      TensorDerivativeImageArrayType;

  /** Typedefs for the multiplication vectors */
  typedef typename Superclass::DeformationVectorImageArrayType
      DeformationVectorImageArrayType;
  typedef typename Superclass::DeformationVectorImageRegionType
      DeformationVectorImageRegionType;
  typedef typename
      RegistrationFunctionType::DeformationVectorImageRegionArrayType
      DeformationVectorImageRegionArrayType;

  /** Normal vector types.  There are three normals at each voxel, which are
   *  stored in a matrix.  If the normals are based on the structure tensor,
   *  then the matrix will be symmetric, but we won't enforce that. */
  typedef double NormalVectorComponentType;
  typedef itk::Vector< NormalVectorComponentType, ImageDimension >
      NormalVectorType;
  typedef itk::Image< NormalVectorType, ImageDimension >
      NormalVectorImageType;
  typedef typename NormalVectorImageType::Pointer
      NormalVectorImagePointer;
  typedef itk::Matrix< NormalVectorComponentType,
                       ImageDimension,
                       ImageDimension >                 NormalMatrixType;
  typedef itk::Image< NormalMatrixType, ImageDimension >
      NormalMatrixImageType;
  typedef typename NormalMatrixImageType::Pointer
      NormalMatrixImagePointer;
  typedef ZeroFluxNeumannBoundaryCondition< NormalMatrixImageType >
      NormalMatrixImageBoundaryConditionType;
  typedef itk::ImageRegionIterator< NormalMatrixImageType >
      NormalMatrixImageRegionType;
  typedef typename NormalMatrixImageType::RegionType
      ThreadNormalMatrixImageRegionType;

  /** Types for weighting between the plane/tube/point states of the
    * regularization - a matrix A, likely symmetric but we won't enforce
    * that here. */
  typedef double                                        WeightComponentType;
  typedef typename itk::Matrix< WeightComponentType,
                                ImageDimension,
                                ImageDimension >        WeightMatrixType;
  typedef itk::Image< WeightMatrixType, ImageDimension >
      WeightMatrixImageType;
  typedef typename WeightMatrixImageType::Pointer
      WeightMatrixImagePointer;
  typedef itk::ImageRegionIterator< WeightMatrixImageType >
      WeightMatrixImageRegionType;
  typedef typename WeightMatrixImageType::RegionType
      ThreadWeightMatrixImageRegionType;

  /** We also need a weighting w between the diffusive regularization and the
    * anisotropic regularization, which is a scalar. */
  typedef itk::Image< WeightComponentType, ImageDimension >
      WeightComponentImageType;
  typedef typename WeightComponentImageType::Pointer
      WeightComponentImagePointer;
  typedef itk::ImageRegionIterator< WeightComponentImageType >
      WeightComponentImageRegionType;
  typedef typename WeightComponentImageType::RegionType
      ThreadWeightComponentImageRegionType;

  /** Organ boundary surface types */
  typedef vtkPolyData                                   BorderSurfaceType;
  typedef vtkSmartPointer< BorderSurfaceType >          BorderSurfacePointer;

  /** Tube spatial object types */
  typedef typename itk::SpatialObject< ImageDimension >::ChildrenListType
      TubeListType;
  typedef TubeListType *                                TubeListPointer;
  typedef itk::VesselTubeSpatialObject< ImageDimension >
      TubeType;
  typedef typename TubeType::PointListType              TubePointListType;
  typedef typename TubeType::TubePointType              TubePointType;

  /** The number of div(Tensor \grad u)v terms we sum for the regularizer.
   *  Reimplement in derived classes. */
  virtual int GetNumberOfTerms() const
    { return 4; }

  /** Set/get the organ boundary polydata, which must be in the same space as
   *  the fixed image.  Border normals are computed on this polydata, so it
   *  may be changed over the course of the registration. */
  virtual void SetBorderSurface( BorderSurfaceType * border )
    { m_BorderSurface = border; }
  virtual BorderSurfaceType * GetBorderSurface() const
    { return m_BorderSurface; }

  /** Set/get the list of tube spatial objects, which must be in the same space
   *  as the fixed image. */
  virtual void SetTubeList( TubeListType * tubeList )
    { m_TubeList = tubeList; }
  virtual TubeListType * GetTubeList() const
    { return m_TubeList; }

  /** Set/get the lambda that controls the exponential decay used to calculate
   *  the weight value w as a function of the distance to the closest border
   *  point.  Must be negative. */
  void SetLambda( WeightComponentType l )
    { if ( l < 0 ) { m_Lambda = l; } }
  WeightComponentType GetLambda() const
    { return m_Lambda; }

  /** Set/get the image of the normal vectors.  Setting the normal vector
   * image overrides the border surface polydata if a border surface was
   * also supplied. */
  virtual void SetNormalMatrixImage( NormalMatrixImageType * normalImage )
    { m_NormalMatrixImage = normalImage; }
  virtual NormalMatrixImageType * GetNormalMatrixImage() const
    { return m_NormalMatrixImage; }
  virtual NormalMatrixImageType * GetHighResolutionNormalMatrixImage() const
    { return m_HighResolutionNormalMatrixImage; }
  /** Get the image of a specific normal vector (column of the normal matrix).
   *  Pointer should already have been initialized with New() */
  virtual void GetHighResolutionNormalVectorImage
      ( NormalVectorImagePointer & normalImage,
        int dim,
        bool getHighResolutionNormalVectorImage ) const;

  /** Set/get the weighting matrix A image.  Setting the weighting matrix image
   * overrides the structure tensor eigen analysis. */
  virtual void SetWeightStructuresImage( WeightMatrixImageType * weightImage )
    { m_WeightStructuresImage = weightImage; }
  virtual WeightMatrixImageType * GetWeightStructuresImage() const
    { return m_WeightStructuresImage; }
  virtual WeightMatrixImageType * GetHighResolutionWeightStructuresImage() const
    { return m_HighResolutionWeightStructuresImage; }

  /** Set/get the weighting value w image.  Setting the weighting component
    * image overrides the border surface polydata and lambda if the border
    * surface was also supplied. */
  virtual void SetWeightRegularizationsImage(
      WeightComponentImageType * weightImage )
    { m_WeightRegularizationsImage = weightImage; }
  virtual WeightComponentImageType * GetWeightRegularizationsImage() const
    { return m_WeightRegularizationsImage; }
  virtual WeightComponentImageType *
      GetHighResolutionWeightRegularizationsImage() const
    { return m_HighResolutionWeightRegularizationsImage; }

  /** Get the normal components of the deformation field.  The normal
   *  deformation field component images are the same for both the SMOOTH_NORMAL
   *  and PROP_NORMAL terms, so we will return one arbitrarily. */
  virtual const DeformationFieldType * GetNormalDeformationComponentImage()
      const
    {
    return this->GetDeformationComponentImage( SMOOTH_NORMAL );
    }

protected:
  AnisotropicDiffusiveSparseRegistrationFilter();
  virtual ~AnisotropicDiffusiveSparseRegistrationFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Handy for array indexing. */
  enum DivTerm { SMOOTH_TANGENTIAL,
                 SMOOTH_NORMAL,
                 PROP_TANGENTIAL,
                 PROP_NORMAL };

  /** Allocate the deformation component images and their derivative images.
   *  (which may be updated throughout the registration). Reimplement in derived
   *  classes. */
  virtual void InitializeDeformationComponentAndDerivativeImages();

  /** Allocate and populate the diffusion tensor images.
   *  Reimplement in derived classes. */
  virtual void ComputeDiffusionTensorImages();

  /** Allocate and populate the images of multiplication vectors that the
   *  div(T \grad(u)) values are multiplied by.  Allocate and populate all or
   *  some of the multiplication vector images in derived classes.  Otherwise,
   *  default to e_l, where e_l is the lth canonical unit vector. */
  virtual void ComputeMultiplicationVectorImages();

  /** Updates the deformation vector component images on each iteration. */
  virtual void UpdateDeformationComponentImages();

  /** Computes the first- and second-order partial derivatives of the
   *  deformation component images on each iteration.  Override in derived
   *  classes if the deformation components image pointers are not unique, to
   *  avoid computing the same derivatives multiple times. */
  virtual void ComputeDeformationComponentDerivativeImages();

  /** If needed, allocates and computes the normal vector and weight images. */
  virtual void SetupNormalMatrixAndWeightImages();

  /** Compute the normals for the border surface. */
  void ComputeBorderSurfaceNormals();

  /** Compute the normals for the tube list. */
  void ComputeTubeNormals();

  /** Provide access to the tube surface for derived classes. */
  virtual BorderSurfaceType * GetTubeSurface() const
    { return m_TubeSurface; }

  /** Computes the normal vector image and weighting factors w given the
   *  surface border polydata. */
  virtual void ComputeNormalMatrixAndWeightImages(
      bool computeNormals,
      bool computeWeightStructures,
      bool computeWeightRegularizations );

  /** Computes the normal vectors and distances to the closest point given
   *  an initialized vtkPointLocator and the surface border normals */
  virtual void GetNormalsAndDistancesFromClosestSurfacePoint(
      bool computeNormals,
      bool computeWeightStructures,
      bool computeWeightRegularizations );

  /** Does the actual work of updating the output over an output region supplied
   *  by the multithreading mechanism.
   *  \sa GetNormalsAndDistancesFromClosestSurfacePoint
   *  \sa GetNormalsAndDistancesFromClosestSurfacePointThreaderCallback */
  virtual void ThreadedGetNormalsAndDistancesFromClosestSurfacePoint(
      vtkPointLocator * surfacePointLocator,
      vtkFloatArray * surfaceNormalData,
      vtkPointLocator * tubePointLocator,
      vtkFloatArray * tubeNormal1Data,
      vtkFloatArray * tubeNormal2Data,
      ThreadNormalMatrixImageRegionType & normalRegionToProcess,
      ThreadWeightMatrixImageRegionType & weightMatrixRegionToProcess,
      ThreadWeightComponentImageRegionType & weightComponentRegionToProcess,
      bool computeNormals,
      bool computeWeightStructures,
      bool computeWeightRegularizations,
      int threadId );

  /** Computes the weighting factor w from the distance to the border.  The
   *  weight should be 1 near the border and 0 away from the border. */
  virtual WeightComponentType ComputeWeightFromDistance(
      const WeightComponentType distance ) const;

private:
  // Purposely not implemented
  AnisotropicDiffusiveSparseRegistrationFilter(const Self&);
  void operator=(const Self&); // Purposely not implemented

  /** Structure for passing information into static callback methods.  Used in
   * the subclasses threading mechanisms. */
  struct AnisotropicDiffusiveSparseRegistrationFilterThreadStruct
    {
    AnisotropicDiffusiveSparseRegistrationFilter * Filter;
    vtkPointLocator * SurfacePointLocator;
    vtkFloatArray * SurfaceNormalData;
    vtkPointLocator * TubePointLocator;
    vtkFloatArray * TubeNormal1Data;
    vtkFloatArray * TubeNormal2Data;
    ThreadNormalMatrixImageRegionType NormalMatrixImageLargestPossibleRegion;
    ThreadWeightMatrixImageRegionType
        WeightStructuresImageLargestPossibleRegion;
    ThreadWeightComponentImageRegionType
        WeightRegularizationsImageLargestPossibleRegion;
    bool ComputeNormals;
    bool ComputeWeightStructures;
    bool ComputeWeightRegularizations;
    };

  /** This callback method uses ImageSource::SplitRequestedRegion to acquire an
   * output region that it passes to
   * ThreadedGetNormalsAndDistancesFromClosestSurfacePoint for processing. */
  static ITK_THREAD_RETURN_TYPE
      GetNormalsAndDistancesFromClosestSurfacePointThreaderCallback(
          void * arg );

  /** Organ boundary surface and surface of border normals */
  BorderSurfacePointer                m_BorderSurface;
  TubeListPointer                     m_TubeList;
  BorderSurfacePointer                m_TubeSurface;

  /** Image storing information we will need for each voxel on every
   *  registration iteration */
  NormalMatrixImagePointer            m_NormalMatrixImage;
  WeightMatrixImagePointer            m_WeightStructuresImage;
  WeightComponentImagePointer         m_WeightRegularizationsImage;

  /** Highest resolution versions of the normal and weight images, useful
   *  to calculate once (setting m_ImageAttributeImage) at the highest
   *  resolution during multiresolution registration, and then resampling on
   *  each scale.  The normal matrix image and weight structures image are
   *  resampled using nearest neighbor, while the weight regularizations image
   *  are resampled using a linear interpolation */
  NormalMatrixImagePointer            m_HighResolutionNormalMatrixImage;
  WeightMatrixImagePointer            m_HighResolutionWeightStructuresImage;
  WeightComponentImagePointer
      m_HighResolutionWeightRegularizationsImage;

  /** The lambda factor for computing the weight from distance.  Weight is
   * modeled as exponential decay: weight = e^(lambda * distance).
   * (lamba must be negative) */
  WeightComponentType                 m_Lambda;
};

} // end namespace itk

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkAnisotropicDiffusiveSparseRegistrationFilter+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkAnisotropicDiffusiveSparseRegistrationFilter.txx"
#endif

#endif
