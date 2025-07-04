import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axiosInstance from '../axiosConfig';
import { toast } from 'react-toastify';

import BasePage from '../components/BasePage';


const UpdateMyInfo = () => {
  const [data, setData]  = useState({});
  
  const navigate= useNavigate();

  useEffect(()=>{
    async function reloadData(){
      try {
        const response = await axiosInstance.get("/users/my-info", {}, {withCredentials: true});
        if(!response.data.success){
              toast.error(response.data.message);
        }else{
            setData(response.data.data[0]);
        }

      } catch (err) {
          console.error("Login error:", err.message);
      }
    }
    
    reloadData();
  }, []);
  
  
  const handleSubmit= async(e) =>{
    try {
        const response = await axiosInstance.put("/users/update", data, {withCredentials: true});
        if(!response.data.success){
            toast.error(response.data.message);
        }else{
            toast.success("Personal Information updated successfully!");
            navigate("/my-account");
        }

    } catch (err) {
      console.error("Login error:", err.message);
    }
  }

const pageContent=()=>{


    return(
      <div className="content-pane">
        <h1 className='content-title'>Update My Personal Info</h1>
        <hr />
        <form action={handleSubmit} className="p-8 w-full" >
          <div className="personal-info-pane">
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Employee ID#:
            </label>
            <input
              id="emplyeeID"
              type="text"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400 bg-blue-100"
              value={data.employeeID}
              readOnly
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              First Name:
            </label>
            <input
              id="firstName"
              type="text"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.firstName}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "firstName": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Middle Name:
            </label>
            <input
              id="middleName"
              type="text"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.middleName}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "middleName": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Last Name:
            </label>
            <input
              id="lastName"
              type="text"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.lastName}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "lastName": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Contact#:
            </label>
            <input
              id="contact"
              type="text"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.contactNumber}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "contactNumber": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Email:
            </label>
            <input
              id="email"
              type="email"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.emailAddress}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "emailAddress": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="newPassword">
              Address:
            </label>
            <textarea
              id="address"
              rows="5"
              cols="40"
              className="w-100 px-4 py-2 border rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-400"
              value={data.address}
              onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "address": e.target.value
              }))}}
              required
            />
            <label className="text-gray-600 mb-1 w-fit" htmlFor="sendSmsNotification">
              Receive SMS Notification for Full SmartBins:
            </label>
          <input 
            className="px-4 py-2 my-auto items-align-center h-10 w-10"
            id="sendSmsNotification"
            type="checkbox"
            checked={data.sendSmsNotification}
            onChange={(e) =>{setData(prevData=>({
                ...prevData,
                "sendSmsNotification": e.target.checked
              }))}}
          />
          </div>
          <div className="flex items-center justify-center w-full h-auto pt-10">
            <button
              type="submit"
              className="w-50 bg-blue-600 text-white py-2 rounded-xl hover:bg-blue-700 transition duration-200"
            >
              submit
            </button>
          </div>
          
              
        </form>
        
      </div>
    );
  }


  return (
    <>
      <BasePage pageTitle="My Account" pageContent={pageContent}/>
    </>
  )
}

export default UpdateMyInfo