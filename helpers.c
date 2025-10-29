#include "voxen.h"

bool ConstIndexInBounds(int constdex) {
	return (constdex >= 0 && constdex <= 760);
}

bool ConstIndexIsGeometry(int constdex) {
	return (constdex >= 0 && constdex <= 306) || constdex == 760;
}

bool ConstIndexIsDoor(int constdex) {
	return (constdex >= 496 && constdex < 515);
}

bool ConstIndexIsLightStaticSaveable(int constdex) {
	return constdex == 748;
}

bool ConstIndexIsGenericTransform(int constdex) {
	return constdex == 749;
}

bool ConstIndexIsDynamicObject(uint16_t constIndex) {
    return     (constIndex >= 307 && constIndex <= 404)
            ||  constIndex == 417
            || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437)
            || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463)
            || (constIndex >= 465 && constIndex <= 476);
}

bool ConstIndexIsStaticObjectSaveable(int constdex) {
	return ((constdex >= 448 && constdex < 458)
			|| constdex == 480 || constdex == 516
			|| (constdex >= 518 && constdex <= 526)
			|| constdex == 530 || constdex == 531 || constdex == 546
			|| constdex == 555 || constdex == 594 || constdex == 596
			|| constdex == 598
			|| (constdex >= 600 && constdex < 603)
			|| (constdex >= 604 && constdex < 616)
			|| (constdex >= 688 && constdex < 693)
			|| constdex == 694 || constdex == 695
			|| (constdex >= 699 && constdex < 704)
			|| (constdex >= 741 && constdex < 746));
}

bool ConstIndexIsStaticObjectImmutable(int constdex) {
	return ((constdex >= 527 && constdex < 530)
			|| (constdex >= 532 && constdex < 546)
			|| (constdex >= 547 && constdex < 553)
			|| constdex == 554
			|| (constdex >= 556 && constdex < 594)
			|| constdex == 595 || constdex == 597 || constdex == 599
			|| constdex == 601 || constdex == 603
			|| (constdex >= 616 && constdex < 688)
			|| constdex == 693 || constdex == 696 || constdex == 697
			|| constdex == 698
			|| (constdex >= 704 && constdex < 717)
			|| constdex == 720
			|| (constdex >= 733 && constdex < 736)
			|| (constdex >= 737 && constdex < 739)
			|| constdex == 746
			|| constdex == 747
			|| (constdex >= 750 && constdex <= 759));
}

bool ConstIndexIsNPC(int constdex) {
	return (constdex >= 419 && constdex < 448);
}

bool ConstIndexIsHardware(int constdex) {
	return (constdex >= 328) && (constdex <= 339);
}
