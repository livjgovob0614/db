/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.
 *
 * Knowage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Knowage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package it.eng.spagobi.tools.massiveExport.services;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;

import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.commons.bo.UserProfile;
import it.eng.spagobi.commons.services.AbstractSpagoBIAction;
import it.eng.spagobi.commons.utilities.GeneralUtilities;
import it.eng.spagobi.tools.massiveExport.bo.ProgressThread;
import it.eng.spagobi.tools.massiveExport.utils.Utilities;
import it.eng.spagobi.utilities.exceptions.SpagoBIServiceException;

public class DownloadMassiveExportZip extends AbstractSpagoBIAction {

	private final String SERVICE_NAME = "DOWNLOAD_MASSIVE_EXPORT_ZIP";

	// logger component
	private static Logger logger = Logger.getLogger(DownloadMassiveExportZip.class);

	public static final String RANDOM_KEY = "RANDOM_KEY";
	public static final String PROGRESS_THREAD_ID = "PROGRESS_THREAD_ID";
	public static final String FUNCT_CD = "FUNCT_CD";
	public static final String PROGRESS_THREAD_TYPE = "PROGRESS_THREAD_TYPE";

	@Override
	public void doService() {
		logger.debug("IN");

		IEngUserProfile profile = getUserProfile();

		String zipFileName = null;
		String folderLabel = null;
		String progressThreadType = null;
		Integer progressThreadId = null;

		try {

			zipFileName = getAttributeAsString(RANDOM_KEY);
			folderLabel = getAttributeAsString(FUNCT_CD);
			progressThreadType = getAttributeAsString(PROGRESS_THREAD_TYPE);
			progressThreadId = getAttributeAsInteger(PROGRESS_THREAD_ID);

			logger.debug(RANDOM_KEY + ": " + zipFileName);
			logger.debug(FUNCT_CD + ": " + folderLabel);
			logger.debug(PROGRESS_THREAD_ID + ": " + progressThreadId);
			logger.debug(PROGRESS_THREAD_TYPE + ": " + progressThreadType);

			logger.debug("RandomKey = " + zipFileName + " FunctCd = " + folderLabel + " ProgressThreadId = " + progressThreadId);

			File zipFile = null;

			if (ProgressThread.TYPE_MASSIVE_EXPORT.equalsIgnoreCase(progressThreadType)) {
				zipFile = Utilities.getMassiveExportZipFile(folderLabel, zipFileName);
			} else if (ProgressThread.TYPE_MASSIVE_SCHEDULE.equalsIgnoreCase(progressThreadType)) {
				zipFile = Utilities.getMassiveScheduleZipFile((String) ((UserProfile) getUserProfile()).getUserId(), folderLabel, zipFileName);
			}

			logger.debug("found file " + zipFile.getAbsolutePath());

			writeBackToClient(zipFile, null, false, zipFile.getName(), "application/zip");

		} catch (Throwable err) {
			logger.error("Error in retrieving file", err);
			throw new SpagoBIServiceException("Error in retrieving zip file ", err);
		}
	}

	private void manageDownload(File zipFile, String fileExtension, HttpServletResponse response) throws IOException {
		logger.debug("IN");
		try {

			response.setHeader("Content-Disposition", "attachment; filename=\"" + zipFile + "." + fileExtension + "\";");
			byte[] exportContent = "".getBytes();
			FileInputStream fis = null;
			fis = new FileInputStream(zipFile);
			exportContent = GeneralUtilities.getByteArrayFromInputStream(fis);
			response.setContentLength(exportContent.length);
			response.getOutputStream().write(exportContent);
			response.getOutputStream().flush();
			if (fis != null)
				fis.close();
		} finally {
			logger.debug("OUT");
		}
	}

}
