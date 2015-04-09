
/*
 +------------------------------------------------------------------------+
 | Phalcon Framework                                                      |
 +------------------------------------------------------------------------+
 | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
 +------------------------------------------------------------------------+
 | This source file is subject to the New BSD License that is bundled     |
 | with this package in the file docs/LICENSE.txt.                        |
 |                                                                        |
 | If you did not receive a copy of the license and are unable to         |
 | obtain it through the world-wide-web, please send an email             |
 | to license@phalconphp.com so we can send you a copy immediately.       |
 +------------------------------------------------------------------------+
 | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
 |          Eduar Carvajal <eduar@phalconphp.com>                         |
 +------------------------------------------------------------------------+
 */

namespace Phalcon;

use Phalcon\DiInterface;
use Phalcon\Security\Exception;
use Phalcon\Di\InjectionAwareInterface;
use Phalcon\Session\AdapterInterface as SessionInterface;

/**
 * Phalcon\Security
 *
 * This component provides a set of functions to improve the security in Phalcon applications
 *
 *<code>
 *	$login = $this->request->getPost('login');
 *	$password = $this->request->getPost('password');
 *
 *	$user = Users::findFirstByLogin($login);
 *	if ($user) {
 *		if ($this->security->checkHash($password, $user->password)) {
 *			//The password is valid
 *		}
 *	}
 *</code>
 */
class Security implements InjectionAwareInterface
{

	protected _dependencyInjector;

	protected _workFactor = 8 { set, get };

	protected _numberBytes = 16;

	protected _csrf;

	/**
	 * Sets the dependency injector
	 *
	 * @param Phalcon\DiInterface $dependencyInjector
	 */
	public function setDI(<DiInterface> dependencyInjector) -> void
	{
		let this->_dependencyInjector = dependencyInjector;
	}

	/**
	 * Returns the internal dependency injector
	 *
	 * @return Phalcon\DiInterface
	 */
	public function getDI() -> <DiInterface>
	{
		return this->_dependencyInjector;
	}

	/**
	 * Sets a number of bytes to be generated by the openssl pseudo random generator
	 *
	 * @param long randomBytes
	 */
	public function setRandomBytes(long! randomBytes) -> void
	{
		let this->_numberBytes = randomBytes;
	}

	/**
	 * Returns a number of bytes to be generated by the openssl pseudo random generator
	 *
	 * @return string
	 */
	public function getRandomBytes() -> string
	{
		return this->_numberBytes;
	}

	/**
	 * Generate a >22-length pseudo random string to be used as salt for passwords
	 *
	 * @return string
	 */
	public function getSaltBytes() -> string
	{
		var safeBytes, numberBytes;

		if !function_exists("openssl_random_pseudo_bytes") {
			throw new Exception("Openssl extension must be loaded");
		}

		let numberBytes = this->_numberBytes;

		loop {

			/**
			 * Produce random bytes using openssl
			 * Filter alpha numeric characters
			 */
			let safeBytes = phalcon_filter_alphanum(base64_encode(openssl_random_pseudo_bytes(numberBytes)));

			if !safeBytes {
				continue;
			}

			if strlen(safeBytes) < 22 {
				continue;
			}

			break;
		}

		return safeBytes;
	}

	/**
	 * Creates a password hash using bcrypt with a pseudo random salt
	 *
	 * @param string password
	 * @param int workFactor
	 * @return string
	 */
	public function hash(string password, int workFactor = 0) -> string
	{
		if !workFactor {
			let workFactor = (int) this->_workFactor;
		}
		return crypt(password, "$2a$" . sprintf("%02s", workFactor) . "$" . this->getSaltBytes());
	}

	/**
	 * Checks a plain text password and its hash version to check if the password matches
	 *
	 * @param string password
	 * @param string passwordHash
	 * @param int maxPasswordLength
	 * @return boolean
	 */
	public function checkHash(string password, string passwordHash, int maxPassLength = 0) -> boolean
	{
		char ch;
		string cryptedHash;
		int i, sum, cryptedLength, passwordLength;

		if maxPassLength {
			if maxPassLength > 0 && strlen(password) > maxPassLength {
				return false;
			}
		}

		let cryptedHash = (string) crypt(password, passwordHash);

		let cryptedLength = strlen(cryptedHash),
        	passwordLength = strlen(passwordHash);

        let cryptedHash .= passwordHash;

        let sum = cryptedLength - passwordLength;
        for i, ch in passwordHash {
        	let sum = sum | (cryptedHash[i] ^ ch);
        }

		return 0 === sum;
	}

	/**
	 * Checks if a password hash is a valid bcrypt's hash
	 *
	 * @param string password
	 * @param string passwordHash
	 * @return boolean
	 */
	public function isLegacyHash(string password, string passwordHash) -> boolean
	{
		return starts_with(passwordHash, "$2a$");
	}

	/**
	 * Generates a pseudo random token key to be used as input's name in a CSRF check
	 *
	 * @param int numberBytes
	 * @return string
	 */
	public function getTokenKey(int numberBytes = null) -> string
	{
		var safeBytes, dependencyInjector, session;

		if !numberBytes {
			let numberBytes = 12;
		}

		if !function_exists("openssl_random_pseudo_bytes") {
			throw new Exception("Openssl extension must be loaded");
		}

		let dependencyInjector = <DiInterface> this->_dependencyInjector;
		if typeof dependencyInjector != "object" {
			throw new Exception("A dependency injection container is required to access the 'session' service");
		}

		let safeBytes = phalcon_filter_alphanum(base64_encode(openssl_random_pseudo_bytes(numberBytes)));
		let session = <SessionInterface> dependencyInjector->getShared("session");
		session->set("$PHALCON/CSRF/KEY$", safeBytes);

		return safeBytes;
	}

	/**
	 * Generates a pseudo random token value to be used as input's value in a CSRF check
	 *
	 * @param int numberBytes
	 * @return string
	 */
	public function getToken(int numberBytes = null) -> string
	{
		var token, dependencyInjector, session;

		if !numberBytes {
			let numberBytes = 12;
		}

		if !function_exists("openssl_random_pseudo_bytes") {
			throw new Exception("Openssl extension must be loaded");
		}

		let token = openssl_random_pseudo_bytes(numberBytes);
		let token = base64_encode(token);
		let token = phalcon_filter_alphanum(token);

		let dependencyInjector = <DiInterface> this->_dependencyInjector;

		if typeof dependencyInjector != "object" {
			throw new Exception("A dependency injection container is required to access the 'session' service");
		}

		let session = <SessionInterface> dependencyInjector->getShared("session");
		session->set("$PHALCON/CSRF$", token);

		return token;
	}

	/**
	 * Check if the CSRF token sent in the request is the same that the current in session
	 *
	 * @param string tokenKey
	 * @param string tokenValue
	 * @return boolean
	 */
	public function checkToken(tokenKey = null, tokenValue = null) -> boolean
	{
		var dependencyInjector, session, request, token;

		let dependencyInjector = <DiInterface> this->_dependencyInjector;

		if typeof dependencyInjector != "object" {
			throw new Exception("A dependency injection container is required to access the 'session' service");
		}

		let session = <SessionInterface> dependencyInjector->getShared("session");

		if !tokenKey {
			let tokenKey = session->get("$PHALCON/CSRF/KEY$");
		}

		if !tokenValue {
			let request = dependencyInjector->getShared("request");

			/**
			 * We always check if the value is correct in post
			 */
			let token = request->getPost(tokenKey);
		} else {
			let token = tokenValue;
		}

		/**
		 * The value is the same?
		 */
		return token == session->get("$PHALCON/CSRF$");
	}

	/**
	 * Returns the value of the CSRF token in session
	 *
	 * @return string
	 */
	public function getSessionToken() -> string
	{
		var dependencyInjector, session;

		let dependencyInjector = <DiInterface> this->_dependencyInjector;

		if typeof dependencyInjector != "object" {
			throw new Exception("A dependency injection container is required to access the 'session' service");
		}

		let session = <SessionInterface> dependencyInjector->getShared("session");
		return session->get("$PHALCON/CSRF$");
	}

	/**
	 * string \Phalcon\Security::computeHmac(string $data, string $key, string $algo, bool $raw = false)
	 *
	 *
	 * @param string data
	 * @param string key
	 * @param string algo
	 * @param boolean raw
	 */
	public function computeHmac(data, key, algo, raw = false)
	{
		var hmac;

		let hmac = hash_hmac(algo, data, key, raw);
		if !hmac {
			throw new Exception("Unknown hashing algorithm: %s" . algo);
		}

		return hmac;
	}
}
