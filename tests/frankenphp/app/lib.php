<?php
function workload(int $iter): string {
    $a = $iter * 2;
    $b = $a + 1;
    $c = "iter={$iter} a={$a} b={$b}";
    return $c;
}
