
SAMPLING
----------
1. Get a training sample in binary file format.
2. Randomise the order of the training sample.
Note: I have separate tools to do these tasks.
IMPORTANT:	Because of issues with batch processing 599 files and numInstances/INSTANCES, the SP randomise feature no longer works.
			Therefore, the input training file must be randomised before running through the SP.


CREATE AN INIT FILE FOR THE TRAINING SAMPLE
--------------------------------------------
Execute: spatial -makeinit


TRAIN THE SP, SAVE THE STATE (and optionally colCodes for the training sample)
-------------------------------------------------------------------------------
Execute: spatial -savestate -codes


READ THE STATE AND GET COLCODES FOR ALL MOVIE PATCH FILES
----------------------------------------------------------
Execute: spatial -allcodes


PATHING ISSUES
-----------------
Refer to lines 172 - 181 to make changes to paths, and number & list of files to process.