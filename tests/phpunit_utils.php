<?php

class IsBinaryEqual extends PHPUnit_Framework_Constraint {
	protected $value;

	public function __construct($value) {
		parent::__construct();

		if (!is_string($value)) {
            throw PHPUnit_Util_InvalidArgumentHelper::factory(1, 'string');
        }

        $this->value = $value;
	}

	private static function toHexString($string) {
		$result = '';
		$len = strlen($string);
		for ($i = 0; $i < $len; $i++) {
			$c = ord($string[$i]);
			$result .= (($c < 16) ? '0' : '') . dechex($c);
		}
		return $result;
	}

	/**
	 *  @return bool
	 *  @throws PHPUnit_Framework_ExpectationFailedException
	 */
    public function evaluate($other, $description = '', $returnResult = false) {
			
		if ($this->value === $other) {
			return true;
		}

		if ($returnResult) {
            return false;
        }

        throw new PHPUnit_Framework_ExpectationFailedException(
            $description . "\n" . 
            	"Expected: " . self::toHexString($this->value) . "\n" .
            	"  Actual: " . self::toHexString($other)
        );
	}

	public function toString() {
		return 'is equal to <text>';
	}
}

function assertBinaryEqual($expected, $actual, $message = '') {
	$constraint = new IsBinaryEqual($expected);
	PHPUnit_Framework_TestCase::assertThat($actual, $constraint, $message);
}