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
package it.eng.spagobi.utilities;

import it.eng.spagobi.utilities.assertion.Assert;
import it.eng.spagobi.utilities.exceptions.SpagoBIRuntimeException;

import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Collection;
import java.util.Date;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.PropertyException;

import net.openhft.hashing.LongHashFunction;

/**
 *
 * Helper class with useful methods.
 *
 */
public class Helper {

	/**
	 * ISO format
	 */

	private Helper() {

	}

	/**
	 * Check if the value is not null and not empty (after a trim)
	 *
	 * @param value
	 * @param name
	 */
	public static void checkNotNullNotTrimNotEmpty(String value, String name) {
		if (value == null) {
			throw new IllegalArgumentException(String.format("%s must not be null.", name));
		}

		if (value.trim().length() != value.length()) {
			throw new IllegalArgumentException(String.format("%s must not contain trailing/leading spaces.", name));
		}

		if (value.isEmpty()) {
			throwEmpty(name);
		}

	}

	private static void throwEmpty(String name) {
		throw new IllegalArgumentException(String.format("%s must not be empty.", name));
	}

	/**
	 * check if value is not negative
	 *
	 * @param value
	 * @param name
	 */
	public static void checkNotNegative(double value, String name) {
		if (value < 0) {
			throwNotNegative(name);
		}

	}

	/**
	 * check if value is not negative
	 *
	 * @param value
	 * @param name
	 */
	public static void checkNotNegative(long value, String name) {
		if (value < 0) {
			throwNotNegative(name);
		}

	}

	private static void throwNotNegative(String name) {
		throw new IllegalArgumentException(String.format("%s must not be negative.", name));
	}

	/**
	 * check if o is not null
	 *
	 * @param o
	 * @param name
	 */
	public static void checkNotNull(Object o, String name) {
		if (o == null) {
			throw new IllegalArgumentException(String.format("%s must not be null.", name));
		}
	}

	/**
	 * check if from is before to
	 *
	 * @param from
	 * @param to
	 * @param fromName
	 * @param toName
	 */
	public static void checkGreater(Date from, Date to, String fromName, String toName) {
		if (from.getTime() >= to.getTime()) {
			throwGreater(fromName, toName);
		}
	}

	/**
	 * check if b is greater than a
	 *
	 * @param a
	 * @param b
	 * @param aName
	 * @param bName
	 */
	public static void checkGreater(long a, long b, String aName, String bName) {
		if (a >= b) {
			throwGreater(aName, bName);
		}
	}

	private static void throwGreater(String fromName, String toName) {
		throw new IllegalArgumentException(String.format("%s must be greather than %s.", toName, fromName));
	}

	/**
	 * check if value is positive, greater than 0
	 *
	 * @param value
	 * @param name
	 */
	public static void checkPositive(double value, String name) {
		if (value <= 0) {
			throw new IllegalArgumentException(String.format("%s must be greather than 0.", name));
		}
	}

	/**
	 * check if the collection is not empty
	 *
	 * @param coll
	 * @param name
	 */
	public static void checkNotEmpty(Collection<?> coll, String name) {
		if (coll.isEmpty()) {
			throw new IllegalArgumentException(String.format("%s must be not empty.", name));
		}

	}

	/**
	 * check if the collection is without null values
	 *
	 * @param coll
	 * @param name
	 */
	public static void checkWithoutNulls(Collection<?> coll, String name) {
		for (Object o : coll) {
			if (o == null) {
				throw new IllegalArgumentException(String.format("%s must not contain null items.", name));
			}
		}

	}

	/**
	 * check if the value is not empty (after a trim)
	 *
	 * @param value
	 * @param name
	 */
	public static void checkNotTrimNotEmpty(String value, String name) {
		assert value != null;

		if (value.trim().isEmpty()) {
			throwEmpty(name);
		}

	}

	/**
	 * convert the xml object to a printable {@link String}
	 *
	 * @param xmlE
	 * @return
	 */
	public static String toString(Object xmlE) {
		try {
			JAXBContext context = JAXBContext.newInstance(xmlE.getClass());
			Marshaller marshaller = context.createMarshaller();
			marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
			StringWriter out = new StringWriter();
			marshaller.marshal(xmlE, out);
			String xml = out.toString();
			return xml;
		} catch (PropertyException e) {
			throw new RuntimeException(e);
		} catch (JAXBException e) {
			throw new RuntimeException(e);
		}
	}

	public static void checkArgument(boolean condition, String message) {
		if (!condition) {
			throw new IllegalArgumentException(message);
		}

	}

	public static void checkNotEmpty(String value, String name) {
		Assert.assertTrue(value != null, "value!=null");
		if (value.isEmpty()) {
			throw new IllegalArgumentException(String.format("%s must not be empty", name));
		}

	}

	public static String md5(String res) {
		try {
			byte[] bytesOfMessage = res.getBytes("UTF-8");
			MessageDigest md = getMD5Instance();
			byte[] thedigest = md.digest(bytesOfMessage);
			return new String(thedigest, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			throw new SpagoBIRuntimeException(e);
		} catch (NoSuchAlgorithmException e) {
			throw new SpagoBIRuntimeException(e);
		}
	}

	private static MessageDigest getMD5Instance() throws NoSuchAlgorithmException {
		// MessageDigest is not thread safe and is not particularly expensive to construct, so instance it each time.
		return MessageDigest.getInstance("MD5");
	}

	public static String sha256(String res) {
		try {
			byte[] bytesOfMessage = res.getBytes("UTF-8");
			MessageDigest md = getSHA256Instance();
			byte[] thedigest = md.digest(bytesOfMessage);

			// convert the byte to hex format method 1
			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < thedigest.length; i++) {
				sb.append(Integer.toString((thedigest[i] & 0xff) + 0x100, 16).substring(1));
			}
			return sb.toString();
		} catch (UnsupportedEncodingException e) {
			throw new SpagoBIRuntimeException(e);
		} catch (NoSuchAlgorithmException e) {
			throw new SpagoBIRuntimeException(e);
		}
	}

	private static MessageDigest getSHA256Instance() throws NoSuchAlgorithmException {
		// MessageDigest is not thread safe and is not particularly expensive to construct, so instance it each time.
		return MessageDigest.getInstance("SHA-256");
	}

	public static long xxHash(String res) {
		return LongHashFunction.xx_r39().hashChars(res);
	}

	public static String toNullIfempty(String s) {
		if (s != null && s.isEmpty()) {
			s = null;
		}
		return s;
	}

}
