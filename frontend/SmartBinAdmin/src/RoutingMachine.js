import { useEffect } from "react";
import L from "leaflet";
import "leaflet-routing-machine";
import { useMap } from "react-leaflet";

const RoutingMachine = ({ waypoints }) => {
  const map = useMap();

  useEffect(() => {
    if (!map || waypoints.length < 2) return;

    const routingControl = L.Routing.control({
      waypoints: waypoints.map((point) => L.latLng(point[0], point[1])),
      routeWhileDragging: false,
      addWaypoints: false,
      show: false,
      lineOptions: {
        styles: [{ color: "blue", weight: 4 }],
      },
      createMarker: () => null,
    }).addTo(map);

    return () => map.removeControl(routingControl);
  }, [map, waypoints]);

  return null;
};

export default RoutingMachine;