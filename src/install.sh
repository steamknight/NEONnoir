.KEY destination/A

COPY NEONnoir01:NEONnoir01 TO <destination>
COPY NEONnoir02:NEONnoir02 TO <destination>

JOIN <destination>/NEONnoir01 <destination>/NEONnoir02 AS <destination>/NEONnoir.lha
LHA x <destination>/NEONnoir.lha <destination>/

DELETE <destination>/NEONnoir01
DELETE <destination>/NEONnoir02
